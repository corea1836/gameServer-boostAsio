#pragma once

#include "pch.h"

class IoContext;
class Acceptor;
class SessionFactory;
class GameLoop;

class Managers {
    public:
        static Managers &Instance() {
            static Managers instance;
            return instance;
        }

        // 초기화
        void Init(uint16 port);

        // Getter
        static IoContext &IoContextManager();
        static SessionFactory &SessionManager();
        static Acceptor &AcceptorManager();
        static GameLoop &GameManager();

    public:
        Managers(const Managers &) = delete;
        Managers &operator=(const Managers &) = delete;

    private:
        Managers();
        ~Managers();

        void InitPort(uint16 port);
        void InitIoContext();
        void InitSessionManager();
        void InitAcceptor();
        void InitGameManager();

    private:
        unique_ptr<IoContext> _ioContext;
        unique_ptr<SessionFactory> _sessionManager;
        unique_ptr<Acceptor> _acceptor;
        shared_ptr<GameLoop> _gameManager;

    private:
        uint16 _port = 0;
        Atomic<bool> _ready;
};
