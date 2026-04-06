#pragma once

#include "BoostForward.h"
#include <boost/asio.hpp>

using boost_tcp_socket = boost::asio::ip::tcp::socket;
using boost_work_guard =
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
