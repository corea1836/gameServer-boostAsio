#pragma once

#include "Types.h"
#include <memory>

class SendBuffer;
using SendBufferRef = std::shared_ptr<SendBuffer>;

class ISession
{
    public:
        virtual ~ISession() = default;
        virtual bool Connect() = 0;
        virtual void Disconnect() = 0;

        virtual void Recv() = 0;
        virtual void Send(SendBufferRef SendBuffer) = 0;

        // getter
    public:
        virtual bool IsConnected() = 0;
        virtual uint16 GetId() = 0;
};
