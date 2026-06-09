#include "Acceptor.h"
#include "Session.h"
#include <iostream>

Acceptor::Acceptor(uint16 port, boost_io_context &ioContext,
                   SessionCreateFunc createFunc)
    : _acceptor(ioContext, boost_tcp::endpoint(boost_tcp::v4(), port)),
      _createFunc(createFunc)
{
    _ready.store(true);
}

void Acceptor::Start()
{
    if (!_ready.load())
    {
        std::cout << "ACCEPTOR_NOT_READY";
        return;
    }

    AcceptAsnyc();
}

void Acceptor::AcceptAsnyc()
{
    SessionRef session = _createFunc();
    _acceptor.async_accept(session->GetSocket(),
                           [this, session](boost_error_code errorCode)
                           {
                               if (!errorCode)
                               {
                                   std::cout << "New Client Connected"
                                             << std::endl;

                                   session->Connect();
                               }

                               AcceptAsnyc();
                           });
}
