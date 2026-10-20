#pragma once

#include "BoostTypes.h"
#include "RecvBuffer.h"
#include "Types.h"
#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <vector>

class Session : public std::enable_shared_from_this<Session>
{
        enum
        {
            MAX_SEND_QUEUE_SIZE = 65535 * 5,
            MAX_INFLIGHT_SIZE = 65535,
        };

    public:
        Session(int64 id);
        virtual ~Session();

        bool Connect();
        void Disconnect();
        void HandleError(boost_error_code errorCode);
        void Recv();
        void Send(SendBufferRef InBuffer);

        bool IsConnected() { return _isConnected; }
        int64 GetId() { return _id; }
        boost_tcp::socket &GetSocket() { return _socket; }
        SessionRef GetSession() { return shared_from_this(); }

    protected:
        virtual bool OnRead(byte *readPos, int32 readSize) = 0;
        virtual void OnConnected() = 0;
        virtual void OnDisconnected() = 0;

    private:
        void OnRecv(int32 recvSize);
        void Write();
        void DoDisconnect();

        int64 _id = 0;
        boost_tcp::socket _socket;
        std::atomic<bool> _isConnected = false;

        RecvBuffer _recvBuffer;

        std::mutex _lock;
        std::deque<SendBufferRef> _sendQueue;
        int32 _sendQueueSize = 0;
        std::atomic<bool> _isWriting = false;
        std::deque<SendBufferRef> _writeQueue;
        std::vector<boost::asio::const_buffer> _inflightBuffer;
};
