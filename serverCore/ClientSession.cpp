// ClientSession.cpp
#include "ClientSession.h"
#include "PacketHandler.h"
#include <__assertion_handler>
#include <chrono>
#include <thread>

ClientSession::ClientSession(tcp::socket socket, int clientId)
    : _socket(std::move(socket)),
      _clientId(clientId),
      _isSending(false) {}

void ClientSession::Start() {
    cout << "[Client " << _clientId << "] 세션 시작\n";
    DoRead();
}

void ClientSession::Send(SendBufferRef sendBuffer) {
    bool shouldStart = false;

    {
        WRITE_LOCK;
        _sendQueue.push_back(sendBuffer);

        if (!_isSending) {
            _isSending = true;
            shouldStart = true;
        }
    }

    if (shouldStart) {
        auto self = shared_from_this();
        boost::asio::post(_socket.get_executor(),
                          [this, self]() { DoWrite(); });
    }
}

void ClientSession::DoRead() {
    auto self = shared_from_this();
    _socket.async_read_some(
        boost::asio::buffer(_recvBuffer),
        [this, self](boost::system::error_code ec, size_t length) {
            if (!ec) {
                OnRecv(_recvBuffer.data(), length);
                DoRead();
            } else {
                cout << "[Client " << _clientId << "] 연결 종료\n";
            }
        });
}

void ClientSession::DoWrite() {
    {
        WRITE_LOCK;

        if (_sendQueue.empty()) {
            _isSending = false;
            return;
        }

        _sendingBuffers.clear();
        _sendingBuffers.swap(_sendQueue);
    }

    std::vector<boost::asio::const_buffer> asioBuffers;
    asioBuffers.reserve(_sendingBuffers.size());

    for (const auto &sendBuf : _sendingBuffers) {
        asioBuffers.push_back(
            boost::asio::buffer(sendBuf->Buffer(), sendBuf->WriteSize()));
    }

    auto self = shared_from_this();
    boost::asio::async_write(
        _socket, asioBuffers,
        [this, self](boost::system::error_code ec, size_t bytes) {
            _sendingBuffers.clear();

            if (!ec) {
                bool hasMore;
                {
                    WRITE_LOCK;
                    hasMore = !_sendQueue.empty();
                    if (!hasMore)
                        _isSending = false;
                }
                if (hasMore)
                    DoWrite();
            } else {
                WRITE_LOCK;
                _isSending = false;
            }
        });
}

void ClientSession::OnRecv(const char *buffer, size_t length) {
    int32_t processLen = 0;
    int32_t receiveCount = 0;

    cout << "\n=== [Client " << _clientId << "] 수신: " << length
         << " 바이트 ===\n";

    while (true) {
        int32_t dataSize = length - processLen;

        if (dataSize < sizeof(PacketHeader))
            break;

        const PacketHeader *header =
            reinterpret_cast<const PacketHeader *>(buffer + processLen);

        if (dataSize < header->size)
            break;

        cout << "[Client " << _clientId << "] 패킷 #"
             << " - ID: " << header->id << ", 크기: " << header->size << "\n";

        const char *body = buffer + processLen + sizeof(PacketHeader);
        uint16_t bodySize = header->size - sizeof(PacketHeader);

        // ✅ S_Connected 받았을 때만 54321 전송
        if (header->id == PKT_S_CONNECTED) {
            Protocol::S_Connected pkt;
            if (pkt.ParseFromArray(body, bodySize)) {
                cout << "[Client " << _clientId << "] S_Connected 수신\n";
                this_thread::sleep_for(std::chrono::milliseconds(5000));
                Protocol::C_Test packet;
                packet.set_temp(54321);
                auto sendBuffer =
                    PacketHandler::MakeSendBuffer(packet, PKT_C_TEST);
                Send(sendBuffer);
            }
        }

        processLen += header->size;
        receiveCount++;
    }

    cout << "수신 패킷 총 " << receiveCount << " 개 처리" << endl;

    // ❌ 여기 있던 54321 전송 코드 삭제!
    // auto _session = shared_from_this();
    // Protocol::C_Test packet;
    // packet.set_temp(54321);
    // auto sendBuffer = PacketHandler::MakeSendBuffer(packet, PKT_C_TEST);
    // _session->Send(sendBuffer);
}
