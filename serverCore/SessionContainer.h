#pragma once

#include "CoreMacro.h"
#include "Types.h"
#include <memory>
#include <mutex>
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
            std::lock_guard<std::mutex> guard(_lock);

            uint16 sessionId = ++_sessionId;
            std::shared_ptr<T> session = std::make_shared<T>(sessionId);
            return session;
        }
        void AddSession(ISessionRef session);
        void RemoveSession(uint16 sessionId);

    private:
        std::mutex _lock;

        uint16 _sessionId = 0;
        std::unordered_map<uint16, ISessionRef> _sessions;
};
