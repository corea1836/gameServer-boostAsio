// 부하 테스트 봇(1-9).
// 사용법: DummyClient [봇 수=200] [실행 초=0(끝없이)] [host=127.0.0.1] [port=7777]
// 실행 초를 주면 끝날 때 모든 봇이 S_EnterWorld를 받았는지에 따라 종료 코드 0/1을 돌려준다

#include "BotPacketHandler.h"
#include "BotSession.h"
#include "CoreManager.h"
#include "IoContext.h"
#include "ThreadPool.h"
#include <cstdio>
#include <cstdlib>

namespace
{

constexpr int32 THREAD_COUNT = 4;
constexpr int64 STATUS_INTERVAL_MS = 5000;

void PrintStatus(int32 botCount)
{
    std::printf("[Bot] connected=%d/%d entered=%d disconnected=%d spawned=%lld despawned=%lld\n",
                BotStats::SConnected.load(), botCount, BotStats::SEntered.load(),
                BotStats::SDisconnected.load(),
                static_cast<long long>(BotStats::SSpawned.load()),
                static_cast<long long>(BotStats::SDespawned.load()));
    std::fflush(stdout);
}

void AsyncWaitStatus(std::shared_ptr<boost_timer> timer, int32 botCount)
{
    timer->expires_after(std::chrono::milliseconds(STATUS_INTERVAL_MS));
    timer->async_wait(
        [timer, botCount](boost_error_code errorCode)
        {
            if (errorCode)
                return;

            PrintStatus(botCount);
            AsyncWaitStatus(timer, botCount);
        });
}

} // namespace

int main(int argc, char *argv[])
{
    int32 botCount = argc > 1 ? std::atoi(argv[1]) : 200;
    int32 durationSec = argc > 2 ? std::atoi(argv[2]) : 0;
    const char *host = argc > 3 ? argv[3] : "127.0.0.1";
    uint16 port = argc > 4 ? static_cast<uint16>(std::atoi(argv[4])) : 7777;

    BotPacketHandler::Init();
    CoreManager::Instance().Init();
    boost_io_context &ioContext = GIoContext.GetNetworkIoContext();

    boost_tcp::endpoint endpoint(boost::asio::ip::make_address(host), port);

    for (int32 i = 0; i < botCount; i++)
    {
        auto session = GSessionContainer.CreateSession<BotSession>();
        session->GetSocket().async_connect(
            endpoint,
            [session](boost_error_code errorCode)
            {
                if (errorCode)
                {
                    std::printf("[Bot] connect fail %lld: %s\n",
                                static_cast<long long>(session->GetId()),
                                errorCode.message().c_str());
                    return;
                }

                session->Connect();
            });
    }

    auto statusTimer = std::make_shared<boost_timer>(ioContext);
    AsyncWaitStatus(statusTimer, botCount);

    boost_timer stopTimer(ioContext);
    if (durationSec > 0)
    {
        stopTimer.expires_after(std::chrono::seconds(durationSec));
        stopTimer.async_wait(
            [](boost_error_code errorCode)
            {
                if (errorCode)
                    return;

                GIoContext.Stop();
            });
    }

    for (int32 i = 0; i < THREAD_COUNT; i++)
        GThreadPool.Launch([&ioContext]() { ioContext.run(); });

    GThreadPool.Join();

    PrintStatus(botCount);
    // 소멸 순서(세션 ↔ 멈춘 io_context)를 따지지 않도록 정리 없이 바로 끝낸다
    std::_Exit(BotStats::SEntered.load() == botCount ? 0 : 1);
}
