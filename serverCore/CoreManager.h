#pragma once

#include "SessionContainer.h"
#include "Types.h"
#include <atomic>
#include <functional>

#define GThreadPool CoreManager::Instance().GetThreadPool()
#define GIoContext CoreManager::Instance().GetIoContext()
#define GSessionContainer CoreManager::Instance().GetSessionContainer()
#define GAcceptor CoreManager::Instance().GetAcceptor()

class CoreManager
{
    public:
        static CoreManager &Instance()
        {
            static CoreManager instance;
            return instance;
        }
        void Init(uint16 port,
                  std::function<SessionRef(void)> sessionCreateFunc);
        void Start();

        ThreadPool &GetThreadPool();
        IoContext &GetIoContext();
        SessionContainer &GetSessionContainer();
        Acceptor &GetAcceptor();

    private:
        CoreManager();
        ~CoreManager();
        CoreManager(const CoreManager &) = delete;
        CoreManager &operator=(const CoreManager &) = delete;

        uint16 _port = 0;
        std::atomic<bool> _ready = false;

        ThreadPoolUniqueRef _threadPool;
        IoContextUniqueRef _ioContext;
        SessionContainerUniqueRef _sessionContainer;
        AcceptorUniqueRef _acceptor;
};
