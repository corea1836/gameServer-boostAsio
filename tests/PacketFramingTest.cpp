#include <gtest/gtest.h>

#include "PacketFraming.h"
#include "PacketHeader.h"
#include <algorithm>
#include <cstring>
#include <vector>

namespace
{

// 헤더(size, id) + 0으로 채운 payload. size가 헤더보다 작아도 헤더 4바이트는 쓴다
std::vector<byte> MakePacket(uint16 size, uint16 id)
{
    std::vector<byte> bytes(std::max<size_t>(size, sizeof(PacketHeader)), 0);
    PacketHeader header{size, id};
    memcpy(bytes.data(), &header, sizeof(header));
    return bytes;
}

// 소켓 recv 대신 bytes[offset, offset + count)를 버퍼에 써 넣는다
void Receive(RecvBuffer &buffer, const std::vector<byte> &bytes, int32 offset,
             int32 count)
{
    memcpy(buffer.GetWritePos(), bytes.data() + offset, count);
    ASSERT_TRUE(buffer.OnWrite(count));
}

// 넘겨받은 패킷의 id를 순서대로 모은다. 넘겨받은 크기가 헤더의 size와 같은지도 본다
PacketCallback CollectIds(std::vector<uint16> &ids)
{
    return [&ids](byte *packet, int32 packetSize)
    {
        auto *header = reinterpret_cast<PacketHeader *>(packet);
        EXPECT_EQ(packetSize, header->size);
        ids.push_back(header->id);
        return true;
    };
}

} // namespace

TEST(PacketFraming, SplitPacketIsParsedWhenCompleted)
{
    RecvBuffer buffer(4096);
    std::vector<uint16> ids;
    auto packet = MakePacket(10, 1);

    Receive(buffer, packet, 0, 6);
    EXPECT_TRUE(ParsePackets(buffer, CollectIds(ids)));
    EXPECT_TRUE(ids.empty());
    EXPECT_EQ(buffer.GetDataSize(), 6);

    Receive(buffer, packet, 6, 4);
    EXPECT_TRUE(ParsePackets(buffer, CollectIds(ids)));
    EXPECT_EQ(ids, std::vector<uint16>{1});
    EXPECT_EQ(buffer.GetDataSize(), 0);
}

TEST(PacketFraming, MultiplePacketsInOneRecv)
{
    RecvBuffer buffer(4096);
    std::vector<uint16> ids;
    std::vector<byte> stream;
    for (const auto &packet :
         {MakePacket(10, 1), MakePacket(8, 2), MakePacket(12, 3)})
        stream.insert(stream.end(), packet.begin(), packet.end());

    // 세 번째 패킷은 앞 6바이트만 도착한다
    Receive(buffer, stream, 0, 10 + 8 + 6);
    EXPECT_TRUE(ParsePackets(buffer, CollectIds(ids)));
    EXPECT_EQ(ids, (std::vector<uint16>{1, 2}));
    EXPECT_EQ(buffer.GetDataSize(), 6);

    Receive(buffer, stream, 10 + 8 + 6, 6);
    EXPECT_TRUE(ParsePackets(buffer, CollectIds(ids)));
    EXPECT_EQ(ids, (std::vector<uint16>{1, 2, 3}));
    EXPECT_EQ(buffer.GetDataSize(), 0);
}

TEST(PacketFraming, SizeSmallerThanHeaderIsError)
{
    for (uint16 size : {0, 3})
    {
        RecvBuffer buffer(4096);
        std::vector<uint16> ids;
        auto packet = MakePacket(size, 1);

        Receive(buffer, packet, 0, static_cast<int32>(packet.size()));
        EXPECT_FALSE(ParsePackets(buffer, CollectIds(ids))) << "size=" << size;
        EXPECT_TRUE(ids.empty()) << "size=" << size;
    }
}

TEST(PacketFraming, SplitHeaderIsParsedWhenCompleted)
{
    RecvBuffer buffer(4096);
    std::vector<uint16> ids;
    auto packet = MakePacket(10, 1);

    Receive(buffer, packet, 0, 2);
    EXPECT_TRUE(ParsePackets(buffer, CollectIds(ids)));
    EXPECT_TRUE(ids.empty());
    EXPECT_EQ(buffer.GetDataSize(), 2);

    Receive(buffer, packet, 2, 8);
    EXPECT_TRUE(ParsePackets(buffer, CollectIds(ids)));
    EXPECT_EQ(ids, std::vector<uint16>{1});
    EXPECT_EQ(buffer.GetDataSize(), 0);
}

TEST(PacketFraming, StopsWhenCallbackReturnsFalse)
{
    std::vector<uint16> ids;
    RecvBuffer buffer(4096);
    std::vector<byte> stream;
    for (const auto &packet : {MakePacket(10, 1), MakePacket(8, 2), MakePacket(12, 3)}) {
        stream.insert(stream.end(), packet.begin(), packet.end());
    }

    Receive(buffer, stream, 0, 10 + 8 + 12);
    EXPECT_FALSE(ParsePackets(buffer, [&ids](byte *packet, int32 packetSize) {
        auto *header = reinterpret_cast<PacketHeader *>(packet);
        if (header->id == 2) return false;
        ids.push_back(header->id);
        return true;
    }));
    EXPECT_EQ(ids, (std::vector<uint16>{1}));
}