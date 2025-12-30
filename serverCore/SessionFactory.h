#pragma once

#include "Lock.h"
#include "Types.h"
#include "pch.h"
#include <unordered_map>

class SessionFactory {
    public:
        SessionFactory();
        ~SessionFactory();

        SessionRef CreateSession();
        void AddSession(SessionRef session);
        void RemoveSession(SessionRef session);

        vector<SessionRef> Sessions();
        uint16 SessionCounts();

    private:
        USE_LOCK;

        Atomic<uint16> _sessionId = 0;
        unordered_map<uint16, SessionRef> _sessions;
};
