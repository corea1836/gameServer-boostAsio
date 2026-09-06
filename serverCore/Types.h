#pragma once

#include <cstdint>
#include <memory>
#include <string>

using byte = unsigned char;
using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;
using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;
using string = std::string;

#define USING_SHARED_PTR(name) using name##Ref = std::shared_ptr<class name>
USING_SHARED_PTR(Session);
USING_SHARED_PTR(RecvBuffer);
USING_SHARED_PTR(SendBuffer);
USING_SHARED_PTR(Job);

#define USING_UNIQUE_PTR(name)                                                 \
    using name##UniqueRef = std::unique_ptr<class name>
USING_UNIQUE_PTR(ThreadPool);
USING_UNIQUE_PTR(IoContext);
USING_UNIQUE_PTR(SessionContainer);
USING_UNIQUE_PTR(Acceptor);
