#include "Acceptor.h"
#include "Session.h"

Acceptor::Acceptor(uint16 port, boost_io_context &ioContext,
                   SessionFactoryFunc factoryFunc)
    : _acceptor(ioContext, boost_tcp::endpoint(boost_tcp::v4(), port)),
      _factoryFunc(factoryFunc) {
    AcceptAsync();
}

Acceptor::~Acceptor() {}

void Acceptor::AcceptAsync() {
    SessionRef session = _factoryFunc();
    _acceptor.async_accept(session->GetSocket(),
                           [this, session](boost_error_code ec) {
                               if (!ec) {
                                   cout << "New Client Connected" << endl;

                                   session->Connect();
                               } else {
                                   cerr << ec.what() << endl;
                               }

                               AcceptAsync();
                           });
}
