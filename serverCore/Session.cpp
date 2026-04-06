#include "Session.h"
#include "IoContext.h"
#include "Manager.h"
#include <iostream>
#include <ostream>

Session::Session(uint16 id)
    : _id(id),
      _socket(GIoContext.GetNetworkIoContext())
{
}

Session::~Session() {}

bool Session::Connect() { return true; }

void Session::Disconnect()
{
    if (_connected.exchange(false) == false)
        return;

    boost_error_code ec;

    if (_socket.is_open())
    {
        _socket.close(ec);

        if (ec)
        {
            std::cout << "Disconnect Fail " << _id << " " << ec.message()
                      << std::endl;
        }
    }

    std::cout << "Disconnect Success " << _id << std::endl;
}

void Session::OnConnected() {}

void Session::OnDisconnected() {}

void Session::HandleError(boost_error_code errorCode)
{
    switch (errorCode.value())
    {
    case 0:
        break;
    case boost::asio::error::operation_aborted:
        std::cout << "Session Aborted. Disconnect In Progress." << std::endl;
        break;
    default:
        Disconnect();
        break;
    }
}
