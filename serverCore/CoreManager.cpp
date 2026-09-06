#include "CoreManager.h"
#include "Acceptor.h"
#include "CoreMacro.h"
#include "IoContext.h"
#include "SessionContainer.h"
#include "ThreadPool.h"

void CoreManager::Init(uint16 port,
                       std::function<SessionRef(void)> sessionCreateFunc)
{
    if (port == 0)
    {
        throw std::invalid_argument("INVALID_PORT");
    }
    _port = port;
    _threadPool = std::make_unique<ThreadPool>();
    _ioContext = std::make_unique<IoContext>();
    _sessionContainer = std::make_unique<SessionContainer>();
    _acceptor = std::make_unique<Acceptor>(
        port, _ioContext->GetNetworkIoContext(), sessionCreateFunc);

    _ready.store(true);
}

void CoreManager::Start() { _acceptor->Start(); }

ThreadPool &CoreManager::GetThreadPool()
{
    ASSERT_CRASH(_ready && _threadPool);
    return *_threadPool;
}

IoContext &CoreManager::GetIoContext()
{
    ASSERT_CRASH(_ready && _ioContext);
    return *_ioContext;
}

SessionContainer &CoreManager::GetSessionContainer()
{
    ASSERT_CRASH(_ready && _acceptor);
    return *_sessionContainer;
}

Acceptor &CoreManager::GetAcceptor()
{
    ASSERT_CRASH(_ready && _acceptor);
    return *_acceptor;
}

CoreManager::CoreManager()
    : _threadPool(nullptr),
      _ioContext(nullptr),
      _sessionContainer(nullptr),
      _acceptor(nullptr)
{
}

CoreManager::~CoreManager() {}
