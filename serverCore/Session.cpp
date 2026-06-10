#include "Session.h"
#include "BoostTypes.h"
#include "CoreMacro.h"
#include "ISession.h"
#include "IoContext.h"
#include "Manager.h"
#include "RecvBuffer.h"
#include "SendBuffer.h"
#include <cstddef>
#include <iostream>
#include <memory>
#include <mutex>
#include <ostream>
#include <vector>

Session::Session(uint16 id)
    : _id(id),
      _socket(GIoContext.GetNetworkIoContext()),
      _recvBuffer(RecvBuffer(65535))
{
}

Session::~Session() {}

bool Session::Connect()
{
    _isConnected.store(true);
    GSessionContainer.AddSession(GetSession());
    OnConnected();
    Recv();
    return true;
}

void Session::Disconnect()
{
    if (_isConnected.exchange(false) == false)
        return;

    boost_error_code ec;

    if (_socket.is_open())
    {
        _socket.close(ec);

        if (ec)
        {
            std::cout << "Disconnect Fail " << _id << " " << ec.message()
                      << std::endl;
        }
    }

    GSessionContainer.RemoveSession(GetId());

    std::cout << "Disconnect Success " << _id << std::endl;
}

void Session::HandleError(boost_error_code errorCode)
{
    switch (errorCode.value())
    {
    case 0:
        break;
    case boost::asio::error::operation_aborted:
        std::cout << "Session Aborted. Disconnect In Progress." << std::endl;
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

void Session::OnRecv(int32 recvSize)
{
    if (recvSize < 0 || !_recvBuffer.OnWrite(recvSize))
    {
        Disconnect();
        return;
    }

    int32 dataSize = _recvBuffer.GetDataSize();

    int32 readSize = Read(recvSize);

    if (readSize < 0 || readSize > dataSize || !_recvBuffer.OnRead(readSize))
    {
        Disconnect();
        return;
    }

    _recvBuffer.Clean();

    Recv();
}

int32 Session::Read(int32 recvSize)
{
    int32 readSize = 0;
    while (true)
    {
        int32 remainSize = recvSize - readSize;

        if (remainSize <= 0 ||
            remainSize < static_cast<int32>(sizeof(PacketHeader)))
            break;

        PacketHeader *header = reinterpret_cast<PacketHeader *>(
            _recvBuffer.GetReadPos() + readSize);

        if (remainSize < header->size)
            break;

        OnRead(_recvBuffer.GetReadPos() + readSize, header->size);

        readSize += header->size;
    }

    return readSize;
}

void Session::Send(SendBufferRef sendBuffer)
{
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
            _sendQueue.push_back(sendBuffer);
            _sendQueueSize += sendBuffer->GetWriteSize();

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
        Write();
    }
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
        SendBufferRef sendBuffer = _writeQueue[inflightCount];

        if (sendBuffer->GetWriteSize() + inflightSize > MAX_INFLIGHT_SIZE)
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
