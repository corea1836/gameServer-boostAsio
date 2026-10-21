#include "GameSession.h"
#include "GameTypes.h"
#include "PacketHandler.h"
#include "Player.h"
#include "World.h"

// Network
bool GameSession::OnRead(byte *readPos, int32 readSize)
{
    if (IsConnected() == false)
        return false;

    GameSessionRef session = GetGameSession();
    bool handleResult = PacketHandler::HandlePacket(session, readPos, readSize);

    return handleResult;
}

void GameSession::OnConnected() {}

void GameSession::OnDisconnected()
{
    if (_selectedPlayer != nullptr)
    {
        if (_enteredWorld != nullptr) {
            _enteredWorld->LeavePlayer(_selectedPlayer->GetActorInfo().actorId);
            _enteredWorld = nullptr;
        }

        _selectedPlayer = nullptr;
    }

    _players.clear();
}

// Handler
void GameSession::HandleLogin()
{
    // TODO : packet receive and parsing

    // TODO : session send
}

void GameSession::AddPlayer(const PlayerRef player)
{
    _players.push_back(player);
}

bool GameSession::ChoosePlayer(int32 selectedPlayerIndex)
{
    if (selectedPlayerIndex >= 0 && selectedPlayerIndex < _players.size() &&
        _players[selectedPlayerIndex])
    {
        PlayerRef player = _players[selectedPlayerIndex];

        _selectedPlayer = player;
        return true;
    }

    return false;
}
