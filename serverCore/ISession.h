#pragma once

#include "Types.h"
#include <memory>

class ISession
{
    public:
        virtual ~ISession() = default;
        virtual bool Connect() = 0;
        virtual void Disconnect() = 0;

        virtual void OnConnected() = 0;
        virtual void OnDisconnected() = 0;

        // getter
    public:
        virtual bool IsConnected() = 0;
        virtual uint16 GetId() = 0;
};
