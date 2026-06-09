#include "RecvBuffer.h"
#include <cstring>

void RecvBuffer::Clean()
{
    int32 dataSize = GetDataSize();

    if (dataSize == 0)
    {
        _readPos = _writePos = 0;
    }
    else
    {
        if (GetFreeSize() < _recvSize)
        {
            memmove(_buffer.get(), GetReadPos(), dataSize);
            _readPos = 0;
            _writePos = dataSize;
        }
    }
}

bool RecvBuffer::OnRead(int32 numOfBytes)
{
    if (GetDataSize() < numOfBytes)
    {
        return false;
    }

    _readPos += numOfBytes;

    return true;
}

bool RecvBuffer::OnWrite(int32 numOfBytes)
{
    if (GetFreeSize() < numOfBytes)
    {
        return false;
    }

    _writePos += numOfBytes;

    return true;
}
