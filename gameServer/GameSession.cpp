#include "GameSession.h"
#include "GameTypes.h"
#include "PacketHandler.h"
#include "Player.h"
#include "World.h"

// Network
bool GameSession::OnRead(byte *readPos, int32 readSize)
{
    GameSessionRef session = GetGameSession();
    bool handleResult = PacketHandler::HandlePacket(session, readPos, readSize);

    return handleResult;
}

void GameSession::OnConnected() {}

void GameSession::OnDisconnected()
{
    if (_selectdPlayer != nullptr)
    {
        WorldRef world = _selectdPlayer->GetWorld();
        world->LeavePlayer(_selectdPlayer->GetActorInfo().actorId);
        _selectdPlayer = nullptr;
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

void GameSession::ChoosePlayer(int32 selectedPlayerIndex)
{
    if (selectedPlayerIndex >= 0 && selectedPlayerIndex < _players.size() &&
        _players[selectedPlayerIndex])
    {
        PlayerRef player = _players[selectedPlayerIndex];

        _selectdPlayer = player;
    }
}
