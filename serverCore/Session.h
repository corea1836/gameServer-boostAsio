#pragma once

#include "Lock.h"
#include "RecvBuffer.h"
#include "pch.h"

class Session : public enable_shared_from_this<Session> {
        enum {
            BUFFER_SIZE = 0x10000,
        };

    public:
        Session(uint16 id);
        virtual ~Session();

        void DisConnect();
        virtual void OnConnected();
        virtual void OnDisConnected();

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
        // Read
        uint32 ProcessRead(BYTE *readBufferPtr, int32 len);
        void OnRead(BYTE *readBufferPtr, int32 len);
        // Write
        void ProcessWrite();

    private:
        uint16 _id;
        boost_tcp::socket _socket;
        Atomic<bool> _connected = false;

    private:
        // Recv
        RecvBuffer _recvBuffer;

        // Send
        USE_LOCK;
        deque<SendBufferRef> _sendQueue;
        deque<SendBufferRef> _sendingBuffers;
        Atomic<bool> _isSending = false;
};
