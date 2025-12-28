#include "Managers.h"
#include "Acceptor.h"
#include "IoContext.h"
#include "SessionFactory.h"
#include "Types.h"
#include <memory>

Managers::Managers()
    : _ioContext(nullptr),
      _sessionManager(nullptr),
      _acceptor(nullptr),
      _ready(false) {}
Managers::~Managers() { cout << "Managers delete" << endl; }

void Managers::Init(uint16 port) {
    InitPort(port);
    InitIoContext();
    InitSessionManager();
    InitAcceptor();
    _ready.exchange(true);
}

void Managers::InitPort(uint16 port) { _port = port; }

void Managers::InitIoContext() { _ioContext = make_unique<IoContext>(); }

IoContext &Managers::IoContextManager() {
    if (!_ready || !_ioContext) {
    }

    return *_ioContext;
}

void Managers::InitSessionManager() {
    _sessionManager = make_unique<SessionFactory>();
}

SessionFactory &Managers::SessionManager() {
    if (!_ready || !_sessionManager) {
    }

    return *_sessionManager;
}

void Managers::InitAcceptor() {
    _acceptor = make_unique<Acceptor>(
        _port, IoContextManager().GetNetworkIoContext(),
        [this]() -> SessionRef { return SessionManager().CreateSession(); });
}

Acceptor &Managers::AcceptorManager() {
    if (!_ready || !_acceptor) {
    }

    return *_acceptor;
}
