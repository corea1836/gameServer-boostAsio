#include "PacketFraming.h"

#include "PacketHeader.h"

bool ParsePackets(RecvBuffer &buffer, const PacketCallback &onPacket)
{
    while (true)
    {
        if (buffer.GetDataSize() < static_cast<int32>(sizeof(PacketHeader)))
            break;

        PacketHeader *header = reinterpret_cast<PacketHeader *>(
            buffer.GetReadPos());

        if (header->size < sizeof(PacketHeader))
            return false;

        if (buffer.GetDataSize() < header->size)
            break;

        if (!onPacket(buffer.GetReadPos(), header->size))
            return false;

        buffer.OnRead(header->size);
    }

    return true;
}
