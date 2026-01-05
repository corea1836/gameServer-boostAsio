#pragma once

#include "Lock.h"
#include "pch.h"
#include <unordered_map>

class SessionFactory {
    public:
        SessionFactory();
        ~SessionFactory();

        SessionRef CreateSession();
        void RemoveSession(SessionRef session);

    private:
        USE_LOCK;

        uint16 _sessionId = 0;
        unordered_map<uint16, SessionRef> _sessions;
};
