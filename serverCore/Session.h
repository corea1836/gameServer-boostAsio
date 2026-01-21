#pragma once

#include "Lock.h"
#include "RecvBuffer.h"
#include "pch.h"

class Session : public enable_shared_from_this<Session> {
        enum {
            BUFFER_SIZE = 0x10000,
        };

    public:
        Session(uint16 id, boost_io_context &io_context);
        virtual ~Session();

        void Connect();
        void DisConnect();

        void AsyncRecv();
        void OnRecv(int32 numOfBytes);

        void Send(SendBufferRef sendBuffer);
        void AsyncSend();

    public:
        boost_tcp::socket &GetSocket() { return _socket; }
        bool IsConnected() { return _connected; }
        SessionRef GetSessionptr() {
            return static_pointer_cast<Session>(shared_from_this());
        }
        uint16 GetId() { return _id; }

    public:
        void HandleError(boost_error_code ec);

    private:
        uint32 ProcessRead(BYTE *readBufferPtr, int32 len);
        void ProcessWrite();

    protected:
        virtual void OnConnected() {}
        virtual uint32 OnRead(BYTE *readBufferPtr, int32 len) {}
        virtual uint32 OnWrite(int32 len) {}
        virtual void OnDisConnected() {}

    private:
        uint16 _id;
        boost_tcp::socket _socket;
        Atomic<bool> _connected{false};

    private:
        // Recv
        RecvBuffer _recvBuffer;

        // Send
        USE_LOCK;
        deque<SendBufferRef> _sendQueue;
        deque<SendBufferRef> _sendingBuffers;
        Atomic<bool> _isSending{false};
};
