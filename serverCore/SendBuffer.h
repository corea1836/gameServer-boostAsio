#pragma once

#include "pch.h"

class SendBuffer {
    public:
        SendBuffer(uint32 allocSize)
            : _allocSize(allocSize),
              _writeSize(0) {
            _buffer.resize(allocSize);
        }

        ~SendBuffer() {}

        BYTE *Buffer() { return _buffer.data(); }
        uint32 AllocSize() { return _allocSize; }
        uint32 WriteSize() { return _writeSize; }
        void Close(uint32 writeSize) { _writeSize = writeSize; }

    private:
        vector<BYTE> _buffer;
        uint32 _allocSize = 0;
        uint32 _writeSize = 0;
};
