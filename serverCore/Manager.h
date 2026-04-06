#pragma once

#include "SessionContainer.h"
#include "Types.h"
#include <atomic>
#include <memory>

#define GThreadPool Manager::Instance().GetThreadPool()
#define GIoContext Manager::Instance().GetIoContext()
#define GSessionContainer Manager::Instance().GetSessionContainer()
#define GAcceptor Manager::Instance().GetAcceptor()

class Manager
{
    public:
        void Init(uint16 port);
        void Start();

    private:
        Manager();
        ~Manager();
        Manager(const Manager &) = delete;
        Manager &operator=(const Manager &) = delete;

    private:
        uint16 _port = 0;
        std::atomic<bool> _ready = false;

        std::unique_ptr<class ThreadPool> _threadPool;
        std::unique_ptr<class IoContext> _ioContext;
        std::unique_ptr<class SessionContainer> _sessionContainer;
        std::unique_ptr<class Acceptor> _acceptor;

        // getter
    public:
        static Manager &Instance()
        {
            static Manager instance;
            return instance;
        }

        ThreadPool &GetThreadPool();
        IoContext &GetIoContext();
        SessionContainer &GetSessionContainer();
        Acceptor &GetAcceptor();
};
