#pragma once

#include "pch.h"

#pragma pack(push, 1)
struct PacketHeader {
        uint16 size;
        uint16 id;
};
#pragma pack(pop)
