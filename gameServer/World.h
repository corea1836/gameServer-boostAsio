#pragma once

#include "ActorFactory.h"
#include <unordered_map>

class World : public std::enable_shared_from_this<World>
{
    public:
        World();
        ~World();

        void Post(CallbackFunc &&callback);
        void PostAfter(int64 tickAfter, CallbackFunc &&callback);

        WorldRef GetWorld() { return shared_from_this(); }
        bool GetIsClosed() { return _isClosed.load(); }

        PlayerRef FindPlayer(int64 playerId);

        void EnterPlayer(PlayerRef player);
        void LeavePlayer(int64 playerId);

        void HandleMove(PlayerRef player, MoveData movdData);

    private:
        template <typename T> std::shared_ptr<T> Spawn()
        {
            return ActorFactory::Create<T>();
        }

        void LeavePlayer_Internal(int64 playerId);
        void HandleMove_Internal(PlayerRef player, MoveData movdData);

        JobSerializerRef _jobs;
        std::unordered_map<int64, PlayerRef> _players;
        std::atomic<bool> _isClosed = false;
};
