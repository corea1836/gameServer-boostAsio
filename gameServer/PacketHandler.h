#pragma once

#include "GameTypes.h"
#include "protobuf/Protocol.pb.h"
#include <CoreMacro.h>
#include <PacketHeader.h>
#include <SendBuffer.h>
// false 를 반환하면 연결을 끊는다. 아직 준비 안 된 패킷은 처리하지 않고 true 를 반환한다.
using PacketHandlerFunc = std::function<bool(GameSessionRef &, byte *, int32)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX + 1];

bool Handle_C_FindMyPlayers(GameSessionRef &session,
                            Protocol::C_FindMyPlayers &pkt);
bool Handle_C_EnterWorld(GameSessionRef &session, Protocol::C_EnterWorld &pkt);
bool Handle_C_LeaveWorld(GameSessionRef &session, Protocol::C_LeaveWorld &pkt);
bool Handle_C_Move(GameSessionRef &session, Protocol::C_Move &pkt);

enum : uint16
{
    PKT_S_Connected = 1,
    PKT_S_Disconnected = 2,
    PKT_C_FindMyPlayers = 3,
    PKT_S_FindMyPlayers = 4,
    PKT_C_EnterWorld = 5,
    PKT_S_EnterWorld = 6,
    PKT_C_LeaveWorld = 7,
    PKT_S_LeaveWorld = 8,
    PKT_S_Spawn = 9,
    PKT_S_Despawn = 10,
    PKT_C_Move = 11,
    PKT_S_Move = 12,

};

class PacketHandler
{
    public:
        static void Init()
        {
            GPacketHandler[PKT_C_FindMyPlayers] =
                [](GameSessionRef &session, byte *buffer, int32 size)
            {
                return HandlePacket<Protocol::C_FindMyPlayers>(
                    Handle_C_FindMyPlayers, session, buffer, size);
            };

            GPacketHandler[PKT_C_EnterWorld] =
                [](GameSessionRef &session, byte *buffer, int32 size)
            {
                return HandlePacket<Protocol::C_EnterWorld>(
                    Handle_C_EnterWorld, session, buffer, size);
            };

            GPacketHandler[PKT_C_LeaveWorld] =
                [](GameSessionRef &session, byte *buffer, int32 size)
            {
                return HandlePacket<Protocol::C_LeaveWorld>(
                    Handle_C_LeaveWorld, session, buffer, size);
            };

            GPacketHandler[PKT_C_Move] =
                [](GameSessionRef &session, byte *buffer, int32 size)
            {
                return HandlePacket<Protocol::C_Move>(Handle_C_Move, session,
                                                      buffer, size);
            };
        }

        static bool HandlePacket(GameSessionRef &session, byte *buffer,
                                 int32 size)
        {
            PacketHeader *header = reinterpret_cast<PacketHeader *>(buffer);
            const auto &function = GPacketHandler[header->id];
            if (function == nullptr) return false;

            return function(session, buffer, size);
        };

        template <typename T>
        static SendBufferRef MakeSendBuffer(T &pkt, uint16 pktId)
        {
            const uint16 dataSize = pkt.ByteSizeLong();
            const uint16 packetSize = dataSize + sizeof(PacketHeader);

            SendBufferRef sendBuffer = std::make_shared<SendBuffer>(packetSize);

            PacketHeader *header =
                reinterpret_cast<PacketHeader *>(sendBuffer->GetData());
            header->size = packetSize;
            header->id = pktId;

            PacketHeader *headerAfterByte = header + 1;
            byte *bodyPtr = reinterpret_cast<byte *>(headerAfterByte);
            sendBuffer->Enclose(packetSize);

            ASSERT_CRASH(pkt.SerializeToArray(bodyPtr, dataSize));
            return sendBuffer;
        }

    private:
        template <typename PacketType, typename ProcessFunc>
        static bool HandlePacket(ProcessFunc func, GameSessionRef &session,
                                 byte *buffer, int32 size)
        {
            PacketType pkt;

            if (!pkt.ParseFromArray(
                    buffer + sizeof(PacketHeader),
                    size - static_cast<int32>(sizeof(PacketHeader))))
                return false;

            return func(session, pkt);
        }
};
