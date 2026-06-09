#pragma once

#include "BoostTypes.h"
#include "CoreMacro.h"
#include "ISession.h"
#include "RecvBuffer.h"
#include "Types.h"
#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <vector>

class Session;
using SessionRef = std::shared_ptr<Session>;

#pragma pack(push, 1)
struct PacketHeader
{
        uint16 size;
        uint16 id;
};
#pragma pack(pop)

class Session : public ISession, public std::enable_shared_from_this<Session>
{
        enum
        {
            MAX_SEND_QUEUE_SIZE = 65535 * 5,
            MAX_INFLIGHT_SIZE = 65535,
        };

    public:
        Session(uint16 id);
        virtual ~Session();

    public:
        virtual bool Connect() override;
        virtual void Disconnect() override;
        virtual void Recv() override;
        virtual void Send(SendBufferRef InBuffer) override;
        void HandleError(boost_error_code errorCode);

    private:
        void OnRecv(int32 recvSize);
        int32 Read(int32 recvSize);
        void Write();

    protected:
        virtual void OnRead(byte *readPos, int32 readSize) = 0;
        virtual void OnConnected() = 0;
        virtual void OnDisconnected() = 0;

    private:
        uint16 _id = 0;
        boost_tcp::socket _socket;
        std::atomic<bool> _isConnected = false;

    private:
        RecvBuffer _recvBuffer;

        std::mutex _lock;
        std::deque<SendBufferRef> _sendQueue;
        int32 _sendQueueSize = 0;
        std::atomic<bool> _isWriting = false;
        std::deque<SendBufferRef> _writeQueue;
        std::vector<boost::asio::const_buffer> _inflightBuffer;

        // getter
    public:
        virtual bool IsConnected() override { return _isConnected; }
        virtual uint16 GetId() override { return _id; }
        boost_tcp::socket &GetSocket() { return _socket; }

        SessionRef GetSession() { return shared_from_this(); }
};
