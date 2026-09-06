// ClientSession.cpp
#include "ClientSession.h"
#include "BoostTypes.h"
#include "ClientPacketHandler.h"
#include <SendBuffer.h>
#include <__assertion_handler>
#include <chrono>
#include <mutex>
#include <thread>

ClientSession::ClientSession(boost_tcp::socket socket, int clientId)
    : _socket(std::move(socket)),
      _clientId(clientId),
      _isSending(false)
{
}

void ClientSession::Start()
{
    std::cout << "[Client " << _clientId << "] 세션 시작\n";

    DoRead();
}

void ClientSession::Send(SendBufferRef sendBuffer)
{
    bool shouldStart = false;

    {
        std::lock_guard<std::mutex> lockGuard(_lock);
        _sendQueue.push_back(sendBuffer);

        if (!_isSending)
        {
            _isSending = true;
            shouldStart = true;
        }
    }

    if (shouldStart)
    {
        auto self = shared_from_this();
        boost::asio::post(_socket.get_executor(),
                          [this, self]() { DoWrite(); });
    }
}

void ClientSession::DoRead()
{
    auto self = shared_from_this();
    _socket.async_read_some(
        boost::asio::buffer(_recvBuffer),
        [this, self](boost::system::error_code ec, size_t length)
        {
            if (!ec)
            {
                OnRecv(_recvBuffer.data(), length);
                DoRead();
            }
            else
            {
                std::cout << "[Client " << _clientId << "] 연결 종료\n";
            }
        });
}

void ClientSession::DoWrite()
{
    {
        std::lock_guard<std::mutex> lockGuard(_lock);

        if (_sendQueue.empty())
        {
            _isSending = false;
            return;
        }

        _sendingBuffers.clear();
        _sendingBuffers.swap(_sendQueue);
    }

    std::vector<boost::asio::const_buffer> asioBuffers;
    asioBuffers.reserve(_sendingBuffers.size());

    for (const auto &sendBuf : _sendingBuffers)
    {
        asioBuffers.push_back(
            boost::asio::buffer(sendBuf->GetData(), sendBuf->GetWriteSize()));
    }

    auto self = shared_from_this();
    boost::asio::async_write(
        _socket, asioBuffers,
        [this, self](boost::system::error_code ec, size_t bytes)
        {
            _sendingBuffers.clear();

            if (!ec)
            {
                bool hasMore;
                {
                    std::lock_guard<std::mutex> lockGuard(_lock);
                    hasMore = !_sendQueue.empty();
                    if (!hasMore)
                        _isSending = false;
                }
                if (hasMore)
                    DoWrite();
            }
            else
            {
                std::lock_guard<std::mutex> lockGuard(_lock);
                _isSending = false;
            }
        });
}

void ClientSession::OnRecv(char *buffer, size_t length)
{
    int32_t processLen = 0;
    int32_t receiveCount = 0;

    std::cout << "\n=== [Client " << _clientId << "] 수신: " << length
              << " 바이트 ===\n";

    while (true)
    {
        int32_t dataSize = static_cast<int32_t>(length) - processLen;

        if (dataSize < static_cast<int32_t>(sizeof(PacketHeader)))
            break;

        PacketHeader *header =
            reinterpret_cast<PacketHeader *>(buffer + processLen);

        if (dataSize < header->size)
            break;

        // 구조화된 디스패치 — ID 에 맞는 핸들러 자동 호출
        ClientSessionRef self = shared_from_this();
        ClientPacketHandler::HandlePacket(self, buffer + processLen,
                                          header->size);

        processLen += header->size;
        receiveCount++;
    }

    std::cout << "수신 패킷 총 " << receiveCount << " 개 처리" << std::endl;
}
