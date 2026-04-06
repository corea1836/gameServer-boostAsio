#pragma once

#include "BoostTypes.h"
#include "ISession.h"
#include "Types.h"

using boost_tcp_socket = boost::asio::ip::tcp::socket;
using boost_error_code = boost::system::error_code;

class Session : public ISession, public std::enable_shared_from_this<Session>
{

    public:
        Session(uint16 id);
        virtual ~Session();

    public:
        virtual bool Connect() override;
        virtual void Disconnect() override;

        virtual void OnConnected() override;
        virtual void OnDisconnected() override;

        void HandleError(boost_error_code errorCode);

    private:
        uint16 _id;
        boost_tcp_socket _socket;
        std::atomic<bool> _connected = false;

        // getter
    public:
        virtual bool IsConnected() override { return _connected; }
        virtual uint16 GetId() override { return _id; }
        boost_tcp_socket &GetSocket() { return _socket; }
};
