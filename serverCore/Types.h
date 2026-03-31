#pragma once

// include
#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

// using with template
template <typename T> using Vector = std::vector<T>;
template <typename T> using Function = std::function<T>;
template <typename T> using Atomic = std::atomic<T>;

// using
using Byte = unsigned char;
using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;
using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

using Mutex = std::mutex;
using LockGuard = std::lock_guard<Mutex>;
using Thread = std::thread;
