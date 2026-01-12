#pragma once

#include "SendBuffer.h"
#include "pch.h"
#include "protobuf/Protocol.pb.h"

using PacketHandlerFunc = function<bool(SessionRef &, BYTE *, int32)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_C_Test(SessionRef &session, Protocol::C_Test &pkt);

#pragma pack(push, 1)
struct PacketHeader {
        uint16 size;
        uint16 id;
};
#pragma pack(pop)

enum : uint16 {
    PKT_C_TEST = 0,
    PKT_S_CONNECTED = 1,
};

class PacketHandler {
    public:
        static void Init() {
            GPacketHandler[PKT_C_TEST] = [](SessionRef &session,
                                            BYTE *bufferPtr, int32 len) {
                return HandlePacket<Protocol::C_Test>(Handle_C_Test, session,
                                                      bufferPtr, len);
            };
        }

        static bool HandlePacket(SessionRef &session, BYTE *bufferPtr,
                                 int32 len) {
            PacketHeader *header = reinterpret_cast<PacketHeader *>(bufferPtr);
            return GPacketHandler[header->id](session, bufferPtr, len);
        }

        template <typename T>
        static SendBufferRef MakeSendBuffer(T &pkt, uint16 pktId) {
            const uint16 dataSize = pkt.ByteSizeLong();
            const uint16 packetSize = dataSize + sizeof(PacketHeader);

            SendBufferRef sendBuffer = make_shared<SendBuffer>(packetSize);

            PacketHeader *header =
                reinterpret_cast<PacketHeader *>(sendBuffer->Buffer());
            header->size = packetSize;
            header->id = pktId;

            ASSERT_CRASH(pkt.SerializeToArray(&header[1], dataSize));

            sendBuffer->Close(packetSize);

            return sendBuffer;
        }

    private:
        template <typename PacketType, typename ProcessFunc>
        static bool HandlePacket(ProcessFunc func, SessionRef &session,
                                 BYTE *bufferPtr, int32 len) {
            PacketType pkt;
            if (!pkt.ParseFromArray(bufferPtr + sizeof(PacketHeader),
                                    len - sizeof(PacketHeader)))
                return false;

            return func(session, pkt);
        }
};
