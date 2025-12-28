#pragma once

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

    public:
        boost_tcp::socket &GetSocket() { return _socket; }
        bool IsConnected() { return _conntected; }
        SessionRef GetSessionptr() {
            return static_pointer_cast<Session>(shared_from_this());
        }
        uint16 GetId() { return _id; }

    public:
        void HandleError(boost_error_code ec);

    private:
        uint16 _id;
        boost_tcp::socket _socket;
        Atomic<bool> _conntected = false;
};
