#pragma once

#include <atomic>
#include <boost/asio.hpp>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <iostream>
#include <memory>
#include <mutex>

using namespace std;

using BYTE = unsigned char;
using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;
using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

template <typename T> using Atomic = atomic<T>;
using Mutex = mutex;
using LockGuard = lock_guard<Mutex>;

using namespace boost::asio;
using boost_io_context = io_context;
using work_guard = executor_work_guard<io_context::executor_type>;
using boost_tcp = ip::tcp;
using boost_error_code = boost::system::error_code;

#define USING_SHARED_PTR(name) using name##Ref = shared_ptr<class name>;

USING_SHARED_PTR(Session);
USING_SHARED_PTR(SendBuffer);
