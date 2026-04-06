#pragma once

namespace boost::asio
{
class io_context;

namespace ip
{
class tcp;
}
} // namespace boost::asio

namespace boost::system
{
class error_code;
}

using boost_io_context = boost::asio::io_context;
using boost_tcp = boost::asio::ip::tcp;
using boost_error_code = boost::system::error_code;
