#include "PacketHandler.h"
#include "ISession.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_C_Test(SessionRef &session, Protocol::C_Test &pkt)
{
    Protocol::S_Connected sConnectPkt;

    SendBufferRef sendBuffer =
        PacketHandler::MakeSendBuffer(sConnectPkt, PKT_S_CONNECTED);

    session->Send(sendBuffer);

    return true;
}
