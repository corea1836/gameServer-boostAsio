#pragma once

#include "JobSerializer.h"
#include "pch.h"

class GameLoop : public JobSerializer {
    public:
        GameLoop(boost_io_context &io_context)
            : JobSerializer(io_context) {}
        ~GameLoop() {}

        void Broadcast(SendBufferRef sendBuffer);

    private:
        GameLoop(const GameLoop &) = delete;
        GameLoop &operator=(const GameLoop &) = delete;
};
