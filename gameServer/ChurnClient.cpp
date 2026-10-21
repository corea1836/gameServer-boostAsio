// 접속/종료 반복 봇(1-4 세션 close 경쟁 검증. 1-5에도 쓴다).
// 사용법: ChurnClient [동시 봇 수=50] [실행 초=0(끝없이)] [host=127.0.0.1] [port=7777]
// 봇마다 접속 → C_FindMyPlayers + C_EnterWorld(0) → S_EnterWorld를 받으면 → 끊기 → 다시 접속을 반복한다.
// 끊는 방법은 봇 번호 % 4로 정해 두고 바꾸지 않는다.
// 매번 무작위로 고르면 STALL(약 65초)에 걸린 봇이 쌓여서, 몇 초 뒤에는 거의 모든 봇이 STALL에 묶였다
//   CLIENT_CLOSE: 입장 0.2~3초 뒤 봇이 소켓을 닫는다 (서버: network 스레드가 읽기 에러를 받고 끊는다)
//   BAD_PACKET:   입장 0.2~3초 뒤 등록 안 된 id를 보내고 서버가 끊을 때까지 읽는다 (서버: 핸들러가 false → 끊는다)
//   STALL:        입장하면 읽기를 멈춘다. 수신 버퍼를 작게 잡아 두어 서버 송신 큐가 넘치고 game 스레드가 끊는다
//                 (서버: Send 오버플로). STALL_HOLD_MS 뒤에 쌓인 데이터를 읽어 서버가 끊었는지(EOF) 확인한다
//   ENTER_CLOSE:  C_EnterWorld를 보내고 0~MAX_ENTER_CLOSE_MS 안에 닫는다(1-5). 서버가 세션 strand에서 플레이어를
//                 고른 뒤 World 큐에서 입장시키기 전에 끊기를 노린다. S_EnterWorld를 먼저 받아도 그대로 닫는다
// 앞의 셋은 입장이 끝난 뒤에 끊는다. 입장 중 끊기로 서버가 죽으면 1-4 검증이 가려지므로, 1-4만 보려면
// ENTER_CLOSE가 서버를 죽이지 않는 상태(1-5 이후)에서 돌린다.
// STALL 봇에게 서버가 계속 보내려면 다른 봇이 움직여야 한다(DummyClient를 같이 띄운다, scripts/churntest.sh).
// 서버 Session을 쓰지 않고 소켓을 직접 다룬다. 1-4에서 Session을 고치는 동안에도 봇의 동작이 바뀌지 않는다.
// 봇 하나의 소켓과 타이머는 그 봇의 strand 위에서만 만진다

#include "PacketHandler.h"
#include <BoostTypes.h>
#include <PacketFraming.h>
#include <PacketHeader.h>
#include <RecvBuffer.h>
#include <Types.h>
#include <array>
#include <atomic>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <random>
#include <thread>
#include <vector>

namespace
{

constexpr int32 THREAD_COUNT = 2;
constexpr int32 RECV_BUFFER_SIZE = 65535;
constexpr int32 MIN_LIFETIME_MS = 200;
constexpr int32 MAX_LIFETIME_MS = 3000;
// 수신 창을 작게 잡아야 서버의 송신이 빨리 막힌다
constexpr int32 STALL_RECV_BUFFER_SIZE = 4096;
// 서버 송신 큐 상한(Session의 MAX_SEND_QUEUE_SIZE, 약 320KB)과 커널 송신 버퍼(128KB~)가 찰 때까지 기다리는 시간.
// TSan 서버(움직이는 봇 50 + 반복 봇 50)에서는 커널 송신 버퍼가 차는 데만 20초 넘게 걸려 40초로도 넘치지 않을 때가 있었다
constexpr int32 STALL_HOLD_MS = 60000;
// 등록 안 된 id를 보낸 뒤, 또는 STALL 확인 때 서버가 끊기를 기다리는 시간
constexpr int32 CLOSE_WAIT_MS = 5000;
// 로컬 서버는 핸드셰이크를 받고 수 ms 안에 입장시키므로 그 안에 닫는다
constexpr int32 MAX_ENTER_CLOSE_MS = 10;
constexpr int32 RECONNECT_DELAY_MS = 100;
constexpr int32 CONNECT_FAIL_DELAY_MS = 1000;
constexpr int64 STATUS_INTERVAL_MS = 5000;
constexpr uint16 BAD_PACKET_ID = UINT16_MAX;

enum class Mode : int32
{
    CLIENT_CLOSE,
    BAD_PACKET,
    STALL,
    ENTER_CLOSE,
    COUNT,
};

enum class Result : int32
{
    CLIENT_CLOSED,
    ENTER_CLOSED,
    BAD_PACKET_CLOSED,
    BAD_PACKET_NOT_CLOSED,
    STALL_CLOSED,
    STALL_NOT_CLOSED,
    // 봇이 끊기 전에 연결이 끊겼다. 서버가 죽었을 수 있다
    UNEXPECTED_CLOSE,
    CONNECT_FAIL,
    COUNT,
};

const char *RESULT_NAMES[] = {
    "client_closed",    "enter_closed",      "bad_packet_closed", "bad_packet_not_closed",
    "stall_closed",     "stall_not_closed",  "unexpected_close",
    "connect_fail",
};

std::atomic<int64> GResults[static_cast<int32>(Result::COUNT)];
std::atomic<uint32> GNextSeed = 1;

int64 GetResult(Result result)
{
    return GResults[static_cast<int32>(result)].load();
}

void PrintStatus()
{
    std::printf("[Churn]");
    for (int32 i = 0; i < static_cast<int32>(Result::COUNT); i++)
        std::printf(" %s=%lld", RESULT_NAMES[i],
                    static_cast<long long>(GResults[i].load()));
    std::printf("\n");
    std::fflush(stdout);
}

class ChurnBot : public std::enable_shared_from_this<ChurnBot>
{
    public:
        ChurnBot(boost_io_context &ioContext, const boost_tcp::endpoint &endpoint,
                 Mode mode)
            : _ioContext(ioContext),
              _endpoint(endpoint),
              _socket(boost::asio::make_strand(ioContext)),
              _timer(_socket.get_executor()),
              _random(GNextSeed.fetch_add(1)),
              _mode(mode),
              _recvBuffer(RECV_BUFFER_SIZE)
        {
        }

        // delayMs 뒤에 접속한다. 타이머가 strand에 묶여 있어서 그 뒤의 일은 전부 strand 위에서 돈다
        void Start(int32 delayMs)
        {
            _timer.expires_after(std::chrono::milliseconds(delayMs));
            _timer.async_wait(
                [self = shared_from_this()](boost_error_code errorCode)
                {
                    if (!errorCode)
                        self->Connect();
                });
        }

    private:
        void Connect()
        {
            if (_mode == Mode::STALL)
            {
                boost_error_code ec;
                _socket.open(boost_tcp::v4(), ec);
                _socket.set_option(
                    boost_tcp::socket::receive_buffer_size(STALL_RECV_BUFFER_SIZE), ec);
            }

            _socket.async_connect(
                _endpoint,
                [self = shared_from_this()](boost_error_code errorCode)
                {
                    if (errorCode)
                    {
                        self->Finish(Result::CONNECT_FAIL);
                        return;
                    }

                    self->SendHandshake();
                });
        }

        void SendHandshake()
        {
            Protocol::C_FindMyPlayers findPkt;
            Protocol::C_EnterWorld enterPkt;
            enterPkt.set_selected_player_index(0);
            _handshake[0] = PacketHandler::MakeSendBuffer(findPkt, PKT_C_FindMyPlayers);
            _handshake[1] = PacketHandler::MakeSendBuffer(enterPkt, PKT_C_EnterWorld);

            std::array<boost::asio::const_buffer, 2> buffers = {
                buffer(_handshake[0]->GetData(), _handshake[0]->GetWriteSize()),
                buffer(_handshake[1]->GetData(), _handshake[1]->GetWriteSize()),
            };

            boost::asio::async_write(
                _socket, buffers,
                [self = shared_from_this()](boost_error_code errorCode, size_t)
                {
                    if (errorCode)
                    {
                        self->Finish(Result::UNEXPECTED_CLOSE);
                        return;
                    }

                    if (self->_mode == Mode::ENTER_CLOSE)
                        self->WaitTimer(std::uniform_int_distribution<int32>(
                            0, MAX_ENTER_CLOSE_MS)(self->_random));

                    self->Read();
                });
        }

        void Read()
        {
            _socket.async_read_some(
                buffer(_recvBuffer.GetWritePos(), _recvBuffer.GetFreeSize()),
                [self = shared_from_this()](boost_error_code errorCode, size_t recvSize)
                {
                    if (errorCode)
                    {
                        self->OnReadClosed();
                        return;
                    }

                    self->OnRead(static_cast<int32>(recvSize));
                });
        }

        // S_EnterWorld만 찾고 나머지 패킷은 버린다
        void OnRead(int32 recvSize)
        {
            _recvBuffer.OnWrite(recvSize);
            bool parsed = ParsePackets(
                _recvBuffer,
                [this](byte *packet, int32)
                {
                    PacketHeader *header = reinterpret_cast<PacketHeader *>(packet);
                    if (!_entered && header->id == PKT_S_EnterWorld)
                        OnEntered();
                    return true;
                });
            _recvBuffer.Clean();

            if (!parsed)
            {
                Finish(Result::UNEXPECTED_CLOSE);
                return;
            }

            // STALL은 입장하면 확인할 때까지 읽지 않는다
            if (_mode == Mode::STALL && _entered && !_waitingServerClose)
                return;

            Read();
        }

        void OnReadClosed()
        {
            // 봇이 이미 닫아서 취소된 읽기
            if (_finished)
                return;

            if (!_waitingServerClose)
            {
                Finish(Result::UNEXPECTED_CLOSE);
                return;
            }

            Finish(_mode == Mode::BAD_PACKET ? Result::BAD_PACKET_CLOSED
                                             : Result::STALL_CLOSED);
        }

        void OnEntered()
        {
            _entered = true;

            // 핸드셰이크를 보낼 때 건 타이머로 닫는다
            if (_mode == Mode::ENTER_CLOSE)
                return;

            int32 waitMs = _mode == Mode::STALL
                               ? STALL_HOLD_MS
                               : std::uniform_int_distribution<int32>(
                                     MIN_LIFETIME_MS, MAX_LIFETIME_MS)(_random);
            WaitTimer(waitMs);
        }

        void WaitTimer(int32 waitMs)
        {
            _timer.expires_after(std::chrono::milliseconds(waitMs));
            _timer.async_wait(
                [self = shared_from_this()](boost_error_code errorCode)
                {
                    if (!errorCode)
                        self->OnTimer();
                });
        }

        void OnTimer()
        {
            if (_finished)
                return;

            if (_waitingServerClose)
            {
                Finish(_mode == Mode::BAD_PACKET ? Result::BAD_PACKET_NOT_CLOSED
                                                 : Result::STALL_NOT_CLOSED);
                return;
            }

            switch (_mode)
            {
            case Mode::CLIENT_CLOSE:
                Finish(Result::CLIENT_CLOSED);
                return;
            case Mode::ENTER_CLOSE:
                Finish(Result::ENTER_CLOSED);
                return;
            case Mode::BAD_PACKET:
                SendBadPacket();
                break;
            case Mode::STALL:
                // 쌓인 데이터를 읽어 나가다 EOF가 오면 서버가 끊은 것이다
                Read();
                break;
            default:
                break;
            }

            _waitingServerClose = true;
            WaitTimer(CLOSE_WAIT_MS);
        }

        void SendBadPacket()
        {
            _badPacket.size = sizeof(PacketHeader);
            _badPacket.id = BAD_PACKET_ID;

            // 실패하면 읽기 쪽에서 끊긴 것을 본다
            boost::asio::async_write(
                _socket, buffer(&_badPacket, sizeof(_badPacket)),
                [self = shared_from_this()](boost_error_code, size_t) {});
        }

        // 결과를 세고 같은 자리에 새 봇을 띄운다. 이 봇은 남은 핸들러가 끝나면 사라진다
        void Finish(Result result)
        {
            if (_finished)
                return;
            _finished = true;
            GResults[static_cast<int32>(result)].fetch_add(1);

            boost_error_code ec;
            _timer.cancel();
            _socket.close(ec);

            auto next = std::make_shared<ChurnBot>(_ioContext, _endpoint, _mode);
            next->Start(result == Result::CONNECT_FAIL ? CONNECT_FAIL_DELAY_MS
                                                       : RECONNECT_DELAY_MS);
        }

        boost_io_context &_ioContext;
        boost_tcp::endpoint _endpoint;
        boost_tcp::socket _socket;
        boost_timer _timer;
        std::mt19937 _random;
        Mode _mode;

        bool _entered = false;
        bool _waitingServerClose = false;
        bool _finished = false;

        SendBufferRef _handshake[2];
        PacketHeader _badPacket{};
        RecvBuffer _recvBuffer;
};

void AsyncWaitStatus(std::shared_ptr<boost_timer> timer)
{
    timer->expires_after(std::chrono::milliseconds(STATUS_INTERVAL_MS));
    timer->async_wait(
        [timer](boost_error_code errorCode)
        {
            if (errorCode)
                return;

            PrintStatus();
            AsyncWaitStatus(timer);
        });
}

} // namespace

int main(int argc, char *argv[])
{
    int32 botCount = argc > 1 ? std::atoi(argv[1]) : 50;
    int32 durationSec = argc > 2 ? std::atoi(argv[2]) : 0;
    const char *host = argc > 3 ? argv[3] : "127.0.0.1";
    uint16 port = argc > 4 ? static_cast<uint16>(std::atoi(argv[4])) : 7777;

    boost_io_context ioContext;
    boost_tcp::endpoint endpoint(boost::asio::ip::make_address(host), port);

    // 접속 시점을 조금씩 벌린다
    for (int32 i = 0; i < botCount; i++)
    {
        Mode mode = static_cast<Mode>(i % static_cast<int32>(Mode::COUNT));
        std::make_shared<ChurnBot>(ioContext, endpoint, mode)->Start(i * 10);
    }

    auto statusTimer = std::make_shared<boost_timer>(ioContext);
    AsyncWaitStatus(statusTimer);

    boost_timer stopTimer(ioContext);
    if (durationSec > 0)
    {
        stopTimer.expires_after(std::chrono::seconds(durationSec));
        stopTimer.async_wait(
            [&ioContext](boost_error_code errorCode)
            {
                if (!errorCode)
                    ioContext.stop();
            });
    }

    std::vector<std::thread> threads;
    for (int32 i = 0; i < THREAD_COUNT; i++)
        threads.emplace_back([&ioContext]() { ioContext.run(); });
    for (std::thread &thread : threads)
        thread.join();

    PrintStatus();
    // DummyClient처럼 정리 없이 끝낸다. 서버가 죽은 흔적(예상 못 한 끊김, 접속 실패)이 있으면 1
    bool serverHealthy = GetResult(Result::UNEXPECTED_CLOSE) == 0 &&
                         GetResult(Result::CONNECT_FAIL) == 0;
    std::_Exit(serverHealthy ? 0 : 1);
}
