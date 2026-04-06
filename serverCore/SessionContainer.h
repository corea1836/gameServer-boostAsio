#pragma once

#include "CoreMacro.h"
#include "Lock.h"
#include <memory>
#include <unordered_map>

class ISession;
using ISessionRef = std::shared_ptr<ISession>;

class SessionContainer
{
    public:
        SessionContainer();
        ~SessionContainer();

    public:
        template <typename T> std::shared_ptr<T> CreateSession()
        {
            WRITE_LOCK;

            uint16 sessionId = ++_sessionId;
            std::shared_ptr<T> session = std::make_shared<T>(sessionId);
            _sessions.insert({sessionId, session});
            return session;
        }
        void RemoveSession(ISessionRef session);

    private:
        USE_LOCK;

        uint16 _sessionId;
        std::unordered_map<uint16, ISessionRef> _sessions;
};
