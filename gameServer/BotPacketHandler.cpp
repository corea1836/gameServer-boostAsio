#include "BotPacketHandler.h"
#include "PacketHandler.h"
#include <PacketHeader.h>
#include <functional>

namespace
{

using BotPacketHandlerFunc = std::function<bool(BotSessionRef &, byte *, int32)>;
BotPacketHandlerFunc GBotPacketHandler[UINT16_MAX + 1];

template <typename PacketType>
bool Parse(PacketType &pkt, byte *buffer, int32 size)
{
    return pkt.ParseFromArray(buffer + sizeof(PacketHeader),
                              size - static_cast<int32>(sizeof(PacketHeader)));
}

// 받기만 하고 내용은 보지 않는 패킷. 봇은 서버와 같은 머신에서 돌아서,
// 파싱 비용을 줄여야 서버 측정에 끼어드는 CPU가 줄어든다
bool Ignore(BotSessionRef &, byte *, int32) { return true; }

} // namespace

void BotPacketHandler::Init()
{
    GBotPacketHandler[PKT_S_FindMyPlayers] =
        [](BotSessionRef &session, byte *buffer, int32 size)
    {
        Protocol::S_FindMyPlayers pkt;
        if (!Parse(pkt, buffer, size) || pkt.players_size() == 0)
            return false;

        session->OnFindMyPlayers();
        return true;
    };

    GBotPacketHandler[PKT_S_EnterWorld] =
        [](BotSessionRef &session, byte *buffer, int32 size)
    {
        Protocol::S_EnterWorld pkt;
        if (!Parse(pkt, buffer, size))
            return false;

        session->OnEnterWorld();
        return true;
    };

    GBotPacketHandler[PKT_S_LeaveWorld] = Ignore;
    GBotPacketHandler[PKT_S_Spawn] = Ignore;
    GBotPacketHandler[PKT_S_Despawn] = Ignore;
    GBotPacketHandler[PKT_S_Move] = Ignore;
}

bool BotPacketHandler::HandlePacket(BotSessionRef &session, byte *buffer,
                                    int32 size)
{
    PacketHeader *header = reinterpret_cast<PacketHeader *>(buffer);
    const auto &function = GBotPacketHandler[header->id];
    if (function == nullptr)
        return false;

    return function(session, buffer, size);
}
