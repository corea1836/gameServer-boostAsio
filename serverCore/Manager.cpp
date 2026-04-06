#include "Manager.h"
#include "Acceptor.h"
#include "CoreMacro.h"
#include "IoContext.h"
#include "Session.h"
#include "ThreadPool.h"
#include <memory>

Manager::Manager()
    : _threadPool(nullptr),
      _ioContext(nullptr),
      _sessionContainer(nullptr),
      _acceptor(nullptr)
{
}

Manager::~Manager() {}

void Manager::Init(uint16 port)
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
        port, _ioContext->GetNetworkIoContext(), [this]() -> SessionRef
        { return _sessionContainer->CreateSession<Session>(); });

    _ready.store(true);
}

void Manager::Start() { _acceptor->Start(); }

ThreadPool &Manager::GetThreadPool()
{
    ASSERT_CRASH(_ready && _threadPool);
    return *_threadPool;
}

IoContext &Manager::GetIoContext()
{
    ASSERT_CRASH(_ready && _ioContext);
    return *_ioContext;
}

SessionContainer &Manager::GetSessionContainer()
{
    ASSERT_CRASH(_ready && _acceptor);
    return *_sessionContainer;
}

Acceptor &Manager::GetAcceptor()
{
    ASSERT_CRASH(_ready && _acceptor);
    return *_acceptor;
}
