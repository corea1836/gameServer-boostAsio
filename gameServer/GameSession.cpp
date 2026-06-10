#include "GameSession.h"
#include "PacketHandler.h"

void GameSession::OnRead(byte *readPos, int32 readSize)
{
    SessionRef session = GetSession();
    PacketHandler::HandlePacket(session, readPos, readSize);
}

void GameSession::OnConnected() {}

void GameSession::OnDisconnected() {}
