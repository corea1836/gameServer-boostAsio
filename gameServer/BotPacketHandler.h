#pragma once

#include "BotSession.h"

// 봇이 받는 S_ 패킷 디스패치. 등록 안 된 id면 false(연결 끊기)로 서버 규칙(1-3)과 같다
class BotPacketHandler
{
    public:
        static void Init();
        static bool HandlePacket(BotSessionRef &session, byte *buffer, int32 size);
};
