#pragma once

#include "GameTypes.h"
#include <Session.h>
#include <Types.h>

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

        void HandleLogin();

        GameSessionRef GetGameSession()
        {
            return std::static_pointer_cast<GameSession>(shared_from_this());
        }

        PlayerRef GetSelectedPlayer() { return _selectdPlayer; }
        const std::vector<PlayerRef> &GetPlayers() const { return _players; }
        void AddPlayer(const PlayerRef player);
        void ChoosePlayer(int32 selectedPlayerIndex);

    private:
        PlayerRef _selectdPlayer;
        std::vector<PlayerRef> _players;

        int64 _dbId = 0;
};
