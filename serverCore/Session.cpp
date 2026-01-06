#include "Session.h"
#include "IoContext.h"
#include "Managers.h"

Session::Session(uint16 id)
    : _id(id),
      _socket(Managers::Instance().IoContextManager().GetNetworkIoContext()) {}

Session::~Session() {}

void Session::DisConnect() {
    if (_conntected.exchange(true))
        return;

    boost_error_code ec;

    if (_socket.is_open()) {
        _socket.close(ec);

        if (ec) {
            cout << "Disconnect Fail " << _id << " " << ec.message() << endl;
        }
    }

    cout << "Disconnect Success " << _id << endl;
}

void Session::HandleError(boost_error_code ec) {
    switch (ec.value()) {
    case 0:
        break;
    case error::operation_aborted:
        cout << "Session Aborted. Disconnect In Progress." << endl;
        break;
    default:
        DisConnect();
        break;
    }
}

void Session::OnConnected() {
    cout << "Hello I am Session " << _id << "." << endl;
}
