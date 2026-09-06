#include "GameLoop.h"
#include "Player.h"
#include "World.h"
#include <JobSerializer.h>
#include <memory>

GameLoop::GameLoop() { _jobs = std::make_shared<JobSerializer>(); }

void GameLoop::Post(CallbackFunc &&callback)
{
    _jobs->Post(std::move(callback));
}

void GameLoop::PostAfter(int64 tickAfter, CallbackFunc &&callback)
{
    _jobs->PostAfter(tickAfter, std::move(callback));
}

void GameLoop::CreateWorld()
{
    Post(
        [this]()
        {
            int64 id = _counter.fetch_add(1);
            if (_worlds.find(id) != _worlds.end())
                return;
            std::shared_ptr<World> newWorld = std::make_shared<World>();
            _worlds.emplace(id, newWorld);
        });
}

void GameLoop::RemoveWorld(int64 worldId)
{
    Post(
        [this, worldId]()
        {
            auto world = _worlds.find(worldId);
            if (world == _worlds.end())
                return;

            // world->second->Close();
            _worlds.erase(worldId);
        });
}

void GameLoop::EnterWorld(int64 worldId, PlayerRef player)
{
    Post([this, worldId, player]() { EnterWorld_Internal(worldId, player); });
}

void GameLoop::LeaveWorld(PlayerRef player)
{
    Post(
        [player]()
        {
            WorldRef currentWorld = player->GetWorld();
            if (currentWorld == nullptr)
                return;

            currentWorld->Post(
                [currentWorld, playerId = player->GetActorData().actorId]() {});
        });
}

void GameLoop::MoveWorld(int64 worldId, PlayerRef player)
{
    Post(
        [this, worldId, player]()
        {
            WorldRef currentWorld = player->GetWorld();
            if (currentWorld == nullptr || currentWorld->GetIsClosed())
            {
                EnterWorld_Internal(worldId, player);
                return;
            }

            currentWorld->Post(
                [this, worldId, player, currentWorld]()
                {
                    currentWorld->LeavePlayer(player->GetActorData().actorId);
                    EnterWorld(worldId, player);
                });
        });
}

void GameLoop::EnterWorld_Internal(int64 worldId, PlayerRef player)
{
    auto targetWorld = FindWorld_Internal(worldId);
    if (targetWorld == nullptr || targetWorld->GetIsClosed())
        return;

    targetWorld->Post([targetWorld, player]()
                      { targetWorld->EnterPlayer(player); });
}
