#pragma once

#include "Types.h"
#include <mutex>
#include <unordered_map>

class SessionContainer
{
    public:
        SessionContainer();
        ~SessionContainer();

        template <typename T> std::shared_ptr<T> CreateSession()
        {
            std::lock_guard<std::mutex> guard(_lock);

            uint16 sessionId = ++_sessionId;
            std::shared_ptr<T> session = std::make_shared<T>(sessionId);
            return session;
        }

        SessionRef FindSession(int64 sessionId)
        {
            std::lock_guard<std::mutex> guard(_lock);

            auto session = _sessions.find(sessionId);
            return session != _sessions.end() ? session->second : nullptr;
        }

        int32 GetSessionCount()
        {
            std::lock_guard<std::mutex> guard(_lock);

            return static_cast<int32>(_sessions.size());
        }

        void AddSession(SessionRef session);
        void RemoveSession(int64 sessionId);

    private:
        std::mutex _lock;

        int64 _sessionId = 0;
        std::unordered_map<int64, SessionRef> _sessions;
};
