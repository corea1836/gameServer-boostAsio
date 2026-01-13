#include "PacketHandler.h"
#include "GameLoop.h"
#include "Managers.h"
#include "Session.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_C_Test(SessionRef &session, Protocol::C_Test &pkt) {
    cout << "[Session] " << session.get()->GetId() << " Handle C_Test body "
         << pkt.temp() << endl;

    Protocol::S_Connected sConnectPkt;

    auto sendBuffer =
        PacketHandler::MakeSendBuffer(sConnectPkt, PKT_S_CONNECTED);

    Managers::GameManager().DoAsync(
        [sendBuffer]() { Managers::GameManager().Broadcast(sendBuffer); });

    return true;
}
