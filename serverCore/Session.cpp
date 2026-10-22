#include "Session.h"
#include "CoreManager.h"
#include "IoContext.h"
#include "Metrics.h"
#include "PacketFraming.h"
#include "SendBuffer.h"
#include <iostream>
#include <ostream>
#include <boost/asio/strand.hpp>

Session::Session(int64 id)
    : _id(id),
      _socket(boost::asio::make_strand(GIoContext.GetNetworkIoContext())),
      _recvBuffer(RecvBuffer(65535))
{
}

Session::~Session() {}

bool Session::Connect()
{
    boost::asio::post(_socket.get_executor(), [this, session = GetSession()]()
    {
        _isConnected.store(true);
        GSessionContainer.AddSession(session);
        OnConnected();
        Recv();
    });

    return true;
}

void Session::Disconnect()
{
    if (_isConnected.exchange(false) == false)
        return;

    boost::asio::post(_socket.get_executor(), [session = GetSession()]()
    {
        session->Disconnect_Internal();
    });
}

void Session::Disconnect_Internal()
{
    boost_error_code ec;

    if (_socket.is_open())
        _socket.close(ec);

    GSessionContainer.RemoveSession(GetId());

    OnDisconnected();
}

void Session::HandleError(boost_error_code errorCode)
{
    switch (errorCode.value())
    {
    case 0:
        break;
    case boost::asio::error::operation_aborted:
        break;
    default:
        Disconnect();
        break;
    }
}

void Session::Recv()
{
    _socket.async_read_some(
        buffer(_recvBuffer.GetWritePos(), _recvBuffer.GetFreeSize()),
        [session = GetSession()](boost_error_code errorCode, size_t recvSize)
        {
            if (errorCode)
            {
                session->HandleError(errorCode);
                return;
            }

            session->OnRecv(recvSize);
        });
}

void Session::Send(SendBufferRef sendBuffer)
{
    Metrics::SSendPackets.fetch_add(1, std::memory_order_relaxed);
    bool canStartWrite = false;
    bool isOverflow = false;

    {
        std::lock_guard<std::mutex> guard(_lock);
        if (_sendQueueSize + sendBuffer->GetWriteSize() > MAX_SEND_QUEUE_SIZE)
        {
            isOverflow = true;
        }

        if (!isOverflow)
        {
            _sendQueueSize += sendBuffer->GetWriteSize();
            _sendQueue.push_back(std::move(sendBuffer));

            if (_isWriting.exchange(true) == false)
            {
                canStartWrite = true;
            }
        }
    }

    if (isOverflow)
    {
        Disconnect();
        return;
    }

    if (canStartWrite)
    {
        boost::asio::post(_socket.get_executor(), [session = GetSession(), now = std::chrono::steady_clock::now()]()
        {
            Metrics::RecordStrandWait(std::chrono::steady_clock::now() - now);
            session->Write();
        });
    }
}

void Session::OnRecv(int32 recvSize)
{
    if (recvSize < 0 || !_recvBuffer.OnWrite(recvSize))
    {
        Disconnect();
        return;
    }
    Metrics::SRecvBytes.fetch_add(recvSize, std::memory_order_relaxed);

    if (!ParsePackets(_recvBuffer, [this](byte *p, int32 n) { return OnRead(p, n); }))
    {
        Disconnect();
        return;
    }
    _recvBuffer.Clean();

    Recv();
}

void Session::Write()
{

    if (_writeQueue.empty())
    {
        std::lock_guard<std::mutex> guard(_lock);
        _sendQueue.swap(_writeQueue);
        _sendQueueSize = 0;
    }

    _inflightBuffer.clear();

    int32 inflightCount = 0;
    int32 inflightSize = 0;

    while (inflightCount < static_cast<int32>(_writeQueue.size()))
    {
        const SendBufferRef &sendBuffer = _writeQueue[inflightCount];

        if (inflightCount > 0 &&
            sendBuffer->GetWriteSize() + inflightSize > MAX_INFLIGHT_SIZE)
            break;

        _inflightBuffer.push_back(
            buffer(sendBuffer->GetData(), sendBuffer->GetWriteSize()));
        inflightSize += sendBuffer->GetWriteSize();
        inflightCount++;
    }

    boost::asio::async_write(
        _socket, _inflightBuffer,
        [session = GetSession(), inflightCount,
         inflightSize](boost_error_code errorCode, size_t size) mutable
        {
            if (errorCode)
            {
                session->HandleError(errorCode);
                return;
            }
            Metrics::SSentBytes.fetch_add(size, std::memory_order_relaxed);
            Metrics::SWrites.fetch_add(1, std::memory_order_relaxed);

            while (inflightCount-- > 0)
            {
                session->_writeQueue.pop_front();
            }

            if (!session->_writeQueue.empty())
            {
                session->Write();
                return;
            }

            bool hasMore = false;
            {
                std::lock_guard<std::mutex> guard(session->_lock);

                if (session->_sendQueue.empty())
                {
                    session->_isWriting.store(false);
                }
                else
                {
                    hasMore = true;
                }
            }

            if (hasMore)
            {
                session->Write();
            }
        });
}


