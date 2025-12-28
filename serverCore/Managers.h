#pragma once

#include "pch.h"

class IoContext;
class Acceptor;
class SessionFactory;

class Managers {
    public:
        static Managers &Instance() {
            static Managers instance;
            return instance;
        }

        // 초기화
        void Init(uint16 port);

        // Getter
        IoContext &IoContextManager();
        SessionFactory &SessionManager();
        Acceptor &AcceptorManager();

    private:
        Managers(const Managers &) = delete;
        Managers &operator=(const Managers &) = delete;

    private:
        Managers();
        ~Managers();

        void InitPort(uint16 port);
        void InitIoContext();
        void InitSessionManager();
        void InitAcceptor();

    private:
        unique_ptr<IoContext> _ioContext;
        unique_ptr<SessionFactory> _sessionManager;
        unique_ptr<Acceptor> _acceptor;

    private:
        uint16 _port = 0;
        Atomic<bool> _ready;
};
