#pragma once

#include "Types.h"
#include "pch.h"

class IoContext;
using SessionFactoryFunc = function<SessionRef(void)>;

class Acceptor {
    public:
        Acceptor(uint16 port, boost_io_context &ioContext,
                 SessionFactoryFunc func);
        ~Acceptor();

    public:
        void AcceptAsync();

    private:
        boost_tcp::acceptor _acceptor;
        SessionFactoryFunc _factoryFunc;
};
