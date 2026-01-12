#pragma once

#include "Lock.h"
#include "pch.h"

using boost::asio::ip::tcp;

class ClientSession : public std::enable_shared_from_this<ClientSession> {
    public:
        ClientSession(tcp::socket socket, int clientId);

        void Start();
        void Send(SendBufferRef sendBuffer);

        // 패킷 생성 헬퍼
        SendBufferRef MakePacket(uint16_t packetId, const void *data,
                                 uint16_t dataSize);

    private:
        void DoRead();
        void DoWrite();
        void OnRecv(const char *buffer, size_t length);

        tcp::socket _socket;
        int _clientId;

        // Send 관련
        USE_LOCK;
        std::deque<SendBufferRef> _sendQueue;
        std::deque<SendBufferRef> _sendingBuffers;
        bool _isSending;

        // Recv 관련
        std::array<char, 4096> _recvBuffer;
};
