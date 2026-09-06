#pragma once

#include "GameTypes.h"
#include <Types.h>
#include <atomic>
#include <unordered_map>

class GameLoop
{
    public:
        GameLoop();
        ~GameLoop() = default;

        void Post(CallbackFunc &&callback);
        void PostAfter(int64 tickAfter, CallbackFunc &&callback);

        void CreateWorld();
        void RemoveWorld(int64 worldId);

        void EnterWorld(int64 worldId, PlayerRef player);
        void LeaveWorld(PlayerRef player);
        void MoveWorld(int64 worldId, PlayerRef player);

    private:
        void EnterWorld_Internal(int64 worldId, PlayerRef player);
        WorldRef FindWorld_Internal(int64 worldId) const
        {
            auto world = _worlds.find(worldId);
            return world != _worlds.end() ? world->second : nullptr;
        }

        JobSerializerRef _jobs;
        std::unordered_map<int64, WorldRef> _worlds;
        std::atomic<int64> _counter = 0;
};
