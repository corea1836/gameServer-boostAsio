#pragma once

#include "BoostTypes.h"

class IoContext
{
    public:
        IoContext();
        ~IoContext() = default;

        void Stop();

        boost_io_context &GetNetworkIoContext() { return _networkIoContext; }
        boost_io_context &GetGameIoContext() { return _gameIoContext; }

    private:
        IoContext(const IoContext &) = delete;
        IoContext &operator=(const IoContext &) = delete;

        boost_io_context _networkIoContext;
        boost_work_guard _networkWorkGuard;
        boost_io_context _gameIoContext;
        boost_work_guard _gameWorkGuard;
};
