#pragma once

#include "Types.h"

class RecvBuffer
{
    public:
        RecvBuffer(int32 recvSize)
            : _recvSize(recvSize),
              _capacity(_recvSize * 5)
        {
            _buffer = std::make_unique<byte[]>(_capacity);
        }

        void Clean();
        bool OnRead(int32 numOfBytes);
        bool OnWrite(int32 numOfBytes);

        int32 GetDataSize() { return _writePos - _readPos; }
        int32 GetFreeSize() { return _capacity - _writePos; }
        byte *GetReadPos() { return _buffer.get() + _readPos; }
        byte *GetWritePos() { return _buffer.get() + _writePos; }

    private:
        int32 _recvSize = 0;
        int32 _capacity = 0;
        int32 _readPos = 0;
        int32 _writePos = 0;
        std::unique_ptr<byte[]> _buffer;
};
