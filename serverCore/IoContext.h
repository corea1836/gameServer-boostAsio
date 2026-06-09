#pragma once

#include "BoostTypes.h"
#include "Types.h"
#include <memory>

class IoContext
{
    public:
        IoContext();
        ~IoContext() = default;

    public:
        void Stop();

    private:
        IoContext(const IoContext &) = delete;
        IoContext &operator=(const IoContext &) = delete;

    private:
        boost_io_context _networkIoContext;
        boost_work_guard _networkWorkGuard;

        boost_io_context _gameIoContext;
        boost_work_guard _gameWorkGuard;

        // getter
    public:
        boost_io_context &GetNetworkIoContext() { return _networkIoContext; }
        boost_io_context &GetGameIoContext() { return _gameIoContext; }
};
