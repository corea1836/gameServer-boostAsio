#include "Managers.h"
#include "Acceptor.h"
#include "GameLoop.h"
#include "IoContext.h"
#include "SessionFactory.h"

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
    InitGameManager();
    _ready.exchange(true);
}

void Managers::InitPort(uint16 port) { _port = port; }

void Managers::InitIoContext() { _ioContext = make_unique<IoContext>(); }

IoContext &Managers::IoContextManager() {
    auto &instance = Managers::Instance();
    if (!instance._ready || !instance._ioContext) {
    }

    return *instance._ioContext;
}

void Managers::InitSessionManager() {
    _sessionManager = make_unique<SessionFactory>();
}

SessionFactory &Managers::SessionManager() {
    auto &instance = Managers::Instance();
    if (!instance._ready || !instance._sessionManager) {
    }

    return *instance._sessionManager;
}

void Managers::InitAcceptor() {
    _acceptor = make_unique<Acceptor>(
        _port, IoContextManager().GetNetworkIoContext(),
        [this]() -> SessionRef { return SessionManager().CreateSession(); });
}

Acceptor &Managers::AcceptorManager() {
    auto &instance = Managers::Instance();
    if (!instance._ready || !instance._acceptor) {
    }

    return *instance._acceptor;
}

void Managers::InitGameManager() {
    _gameManager = make_shared<GameLoop>(IoContextManager().GetGameIoContext());
}

GameLoop &Managers::GameManager() {
    auto &instance = Managers::Instance();
    if (!instance._ready || !instance._gameManager) {
    }

    return *instance._gameManager;
}
