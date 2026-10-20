#include "CoreManager.h"
#include "Acceptor.h"
#include "CoreMacro.h"
#include "IoContext.h"
#include "SessionContainer.h"
#include "ThreadPool.h"

void CoreManager::Init()
{
    _threadPool = std::make_unique<ThreadPool>();
    _ioContext = std::make_unique<IoContext>();
    _sessionContainer = std::make_unique<SessionContainer>();

    _ready.store(true);
}

void CoreManager::Start(uint16 port, std::function<SessionRef()> sessionCreateFunc)
{
    if (port == 0)
        throw std::invalid_argument("Invalid port number");
    _acceptor = std::make_unique<Acceptor>
    (port, _ioContext->GetNetworkIoContext(), sessionCreateFunc);
    _acceptor->Start();
}

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
    ASSERT_CRASH(_ready && _sessionContainer);
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
