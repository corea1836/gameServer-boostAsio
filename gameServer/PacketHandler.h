#pragma once

#include "CoreMacro.h"
#include "ISession.h"
#include "SendBuffer.h"
#include "Session.h"
#include "Types.h"
#include "protobuf/Protocol.pb.h"
#include <cstdint>
#include <functional>
#include <memory>

using PacketHandlerFunc = std::function<bool(SessionRef &, byte *, int32)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_C_Test(SessionRef &session, Protocol::C_Test &pkt);

enum : uint16
{
    PKT_C_TEST = 0,
    PKT_S_CONNECTED = 1,
};

class PacketHandler
{
    public:
        static void Init()
        {
            GPacketHandler[PKT_C_TEST] =
                [](SessionRef &session, byte *buffer, int32 size)
            {
                return HandlePacket<Protocol::C_Test>(Handle_C_Test, session,
                                                      buffer, size);
            };
        }

        static bool HandlePacket(SessionRef &session, byte *buffer, int32 size)
        {
            PacketHeader *header = reinterpret_cast<PacketHeader *>(buffer);
            return GPacketHandler[header->id](session, buffer, size);
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

            return sendBuffer;
            ASSERT_CRASH(pkt.SerializeToArray(bodyPtr, dataSize));
        }

    private:
        template <typename PacketType, typename ProcessFunc>
        static bool HandlePacket(ProcessFunc func, SessionRef &session,
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
