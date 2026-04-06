#pragma once

#include "Session.h"
#include "Types.h"
#include <atomic>
#include <functional>
#include <memory>

class Session;
using SessionRef = std::shared_ptr<Session>;
using SessionCreateFunc = std::function<SessionRef(void)>;

class Acceptor
{
    public:
        Acceptor(uint16 port, boost_io_context &ioContext,
                 SessionCreateFunc createFunc);
        ~Acceptor() = default;

    public:
        void Start();

    private:
        void AcceptAsnyc();

    private:
        boost_tcp::acceptor _acceptor;
        SessionCreateFunc _createFunc;
        std::atomic<bool> _ready = false;
};
