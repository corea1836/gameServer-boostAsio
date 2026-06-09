#pragma once
#include <boost/asio/buffer.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
#include <boost/system/error_code.hpp>

using boost_io_context = boost::asio::io_context;
using boost_work_guard =
    boost::asio::executor_work_guard<boost_io_context::executor_type>;
using boost_tcp = boost::asio::ip::tcp;
using boost_error_code = boost::system::error_code;
using boost::asio::buffer;
