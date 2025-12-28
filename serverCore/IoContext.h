#pragma once

#include "pch.h"

class IoContext {
    public:
        void Init();
        void Stop();

        boost_io_context &GetNetworkIoContext();
        boost_io_context &GetGameIoContext();

    public:
        IoContext();
        ~IoContext() = default;

    public:
        IoContext(const IoContext &) = delete;
        IoContext &operator=(const IoContext &) = delete;

    private:
        boost_io_context _networkIoContext;
        boost_io_context _gameIoContext;

        unique_ptr<work_guard> _networkWorkGuard;
        unique_ptr<work_guard> _gameWorkguard;
};
