#pragma once

#include "BoostTypes.h"
#include "Types.h"
#include <atomic>
#include <functional>

using SessionCreateFunc = std::function<SessionRef(void)>;

class Acceptor
{
    public:
        Acceptor(uint16 port, boost_io_context &ioContext,
                 SessionCreateFunc createFunc);
        ~Acceptor() = default;

        void Start();

    private:
        void AcceptAsnyc();

        boost_tcp::acceptor _acceptor;
        SessionCreateFunc _createFunc;
        std::atomic<bool> _ready = false;
};
