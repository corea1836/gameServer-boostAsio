#pragma once

#include "ActorFactory.h"
#include "GameTypes.h"
#include <unordered_map>

class FixedTicker;

class World : public std::enable_shared_from_this<World>
{
    public:
        static constexpr int64 TICK_INTERVAL_MS = 50;
        World();
        ~World();

        void Init();

        void Post(CallbackFunc &&callback);

        WorldRef GetWorld() { return shared_from_this(); }
        bool GetIsClosed() { return _isClosed.load(); }

        PlayerRef FindPlayer(int64 playerId);

        void EnterPlayer(PlayerRef player);
        void LeavePlayer(int64 playerId);

        void HandleMove(PlayerRef player, MoveInfo nextMove);

    private:
        template <typename T> std::shared_ptr<T> Spawn()
        {
            return ActorFactory::Create<T>();
        }

        void Tick();
        void UpdatePosition(float deltaTime);
        void MulticastMove(int64 now);

        void LeavePlayer_Internal(int64 playerId);
        void HandleMove_Internal(PlayerRef player, MoveInfo nextMove);

        JobSerializerRef _jobs;
        std::shared_ptr<FixedTicker> _fixedTicker;
        std::unordered_map<int64, PlayerRef> _players;
        std::atomic<bool> _isClosed = false;
};
