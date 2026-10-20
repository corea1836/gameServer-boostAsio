#include <gtest/gtest.h>

#include "PacketHandler.h"
#include <algorithm>
#include <cstring>
#include <vector>

namespace
{
    std::vector<byte> MakePacket(uint16 size, uint16 id)
    {
        std::vector<byte> bytes(std::max<size_t>(size, sizeof(PacketHeader)), 0);
        PacketHeader header {size, id};
        memcpy(bytes.data(), &header, sizeof(header));
        return bytes;
    }
}
TEST(PacketHandler, UnregisteredIdReturnsFalse)
{
    PacketHandler::Init();

    GameSessionRef session;

    std::vector<byte> packet = MakePacket(10, 100);

    EXPECT_FALSE(PacketHandler::HandlePacket(session, packet.data(), static_cast<int32>(packet.size())));
}

TEST(PacketHandler, MaxIdReturnsFalse)
{
    PacketHandler::Init();

    GameSessionRef session;

    std::vector<byte> packet = MakePacket(10, UINT16_MAX);

    EXPECT_FALSE(PacketHandler::HandlePacket(session, packet.data(), static_cast<int32>(packet.size())));
}

TEST(PacketHandler, BrokenBodyReturnsFalse)
{
    PacketHandler::Init();

    GameSessionRef session;

    std::vector<byte> packet = MakePacket(sizeof(PacketHeader) + 1, PKT_C_Move);

    packet[sizeof(PacketHeader)] = 0xFF;

    EXPECT_FALSE(PacketHandler::HandlePacket(session, packet.data(), static_cast<int32>(packet.size())));

}