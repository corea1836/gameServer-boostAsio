#include "ClientPacketHandler.h"
#include "PacketHandler.h"
#include "protobuf/Struct.pb.h"
#include <iostream>

ClientPacketHandlerFunc GClientPacketHandler[UINT16_MAX];

bool Handle_S_EnterGame(ClientSessionRef &session, Protocol::S_EnterWorld &pkt)
{
    std::cout << "[Client] S_EnterGame 수신\n";
}
