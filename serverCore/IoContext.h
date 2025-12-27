#pragma once

#include "pch.h"

class IoContext {
    public:
        void Init();
        void Stop();

        io_context &GetNetworkIoContext();
        io_context &GetGameIoContext();

    public:
        IoContext(const IoContext &) = delete;
        IoContext &operator=(const IoContext &) = delete;

    private:
        IoContext();
        ~IoContext() = default;

    private:
        io_context _networkIoContext;
        io_context _gameIoContext;

        unique_ptr<work_guard> _networkWorkGuard;
        unique_ptr<work_guard> _gameWorkguard;
};
