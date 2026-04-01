#pragma once

// include
#include <atomic>
#include <boost/asio.hpp>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

// using with template
template <typename T> using vector = std::vector<T>;
template <typename T> using function = std::function<T>;
template <typename T> using atomic = std::atomic<T>;

// using
using byte = unsigned char;
using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;
using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

using mutex = std::mutex;
using lock_guard = std::lock_guard<mutex>;
using thread = std::thread;
using boost_io_context = boost::asio::io_context;
using boost_work_guard =
    boost::asio::executor_work_guard<boost_io_context::executor_type>;
using boost_tcp = boost::asio::ip::tcp;
using boost_error_code = boost::system::error_code;
