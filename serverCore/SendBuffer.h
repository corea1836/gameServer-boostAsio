#pragma once

#include "Types.h"
#include <vector>

class SendBuffer
{
    public:
        SendBuffer(int32 capacity)
            : _capacity(capacity)
        {
            _buffer.reserve(_capacity);
        }
        ~SendBuffer() = default;

    private:
        int32 _writeSize = 0;
        int32 _capacity = 0;
        std::vector<byte> _buffer;

    public:
        void Enclose(int32 writeSize) { _writeSize = writeSize; }
        int32 GetWriteSize() { return _writeSize; }
        byte *GetData() { return _buffer.data(); }
};
