#pragma once

#include <Session.h>
#include <memory>

class GameSession;
using GameSessionRef = std::shared_ptr<GameSession>;

class GameSession : public Session
{
    public:
        GameSession(uint16 sessionId)
            : Session(sessionId)
        {
        }
        ~GameSession() override {}

        void OnRead(byte *readPos, int32 readSize) override;
        void OnConnected() override;
        void OnDisconnected() override;

        // getter
    public:
        GameSessionRef GetGameSessionRef()
        {
            return std::static_pointer_cast<GameSession>(shared_from_this());
        }
};
