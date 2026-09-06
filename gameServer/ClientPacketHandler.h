#pragma once

#include "ClientSession.h"
#include "PacketHandler.h"
#include "protobuf/Protocol.pb.h"
#include <PacketHeader.h>
#include <cstdint>
#include <functional>
#include <iostream>

// 클라이언트 세션용 디스패치 테이블
using ClientPacketHandlerFunc =
    std::function<bool(ClientSessionRef &, char *, int32_t)>;
extern ClientPacketHandlerFunc GClientPacketHandler[UINT16_MAX];

// 클라이언트는 서버->클라(S_) 패킷만 처리
bool Handle_S_EnterGame(ClientSessionRef &session, Protocol::S_EnterWorld &pkt);

class ClientPacketHandler
{
    public:
        static void Init()
        {
            // 등록 안 된 ID 가 오면 크래시 대신 경고만
            for (int32_t i = 0; i < UINT16_MAX; i++)
                GClientPacketHandler[i] = HandleInvalid;

            GClientPacketHandler[PKT_S_EnterWorld] =
                [](ClientSessionRef &session, char *buffer, int32_t size)
            {
                return HandlePacket<Protocol::S_EnterWorld>(
                    Handle_S_EnterGame, session, buffer, size);
            };
        }

        static bool HandlePacket(ClientSessionRef &session, char *buffer,
                                 int32_t size)
        {
            PacketHeader *header = reinterpret_cast<PacketHeader *>(buffer);
            return GClientPacketHandler[header->id](session, buffer, size);
        }

    private:
        static bool HandleInvalid(ClientSessionRef &session, char *buffer,
                                  int32_t size)
        {
            PacketHeader *header = reinterpret_cast<PacketHeader *>(buffer);
            std::cout << "[Client] 처리되지 않는 패킷 ID: " << header->id
                      << "\n";
            return false;
        }

        template <typename PacketType, typename ProcessFunc>
        static bool HandlePacket(ProcessFunc func, ClientSessionRef &session,
                                 char *buffer, int32_t size)
        {
            PacketType pkt;
            if (!pkt.ParseFromArray(
                    buffer + sizeof(PacketHeader),
                    size - static_cast<int32_t>(sizeof(PacketHeader))))
                return false;

            return func(session, pkt);
        }
};
