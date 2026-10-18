#pragma once

#include "RecvBuffer.h"
#include "Types.h"
#include <functional>

using PacketCallback = std::function<void(byte *packet, int32 packetSize)>;

// buffer에 쌓인 데이터에서 완성된 패킷을 순서대로 onPacket에 넘기고, 넘긴 만큼
// buffer에서 소비한다. 덜 온 패킷은 buffer에 남겨 둔다.
// 헤더의 size가 헤더 크기보다 작으면 false를 반환한다.
bool ParsePackets(RecvBuffer &buffer, const PacketCallback &onPacket);
