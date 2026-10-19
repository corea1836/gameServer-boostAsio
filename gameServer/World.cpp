#include "World.h"
#include "Actor.h"
#include "CoreManager.h"
#include "GameSession.h"
#include "GameTypes.h"
#include "IoContext.h"
#include "PacketHandler.h"
#include "Player.h"
#include "TimeUtils.h"
#include "Types.h"
#include "protobuf/Struct.pb.h"
#include <JobSerializer.h>
#include <algorithm>

World::World() { _jobs = std::make_unique<JobSerializer>(GIoContext.GetGameIoContext()); }

World::~World() {}

void World::Post(CallbackFunc &&callback) { _jobs->Post(std::move(callback)); }

void World::PostAfter(int64 tickAfter, CallbackFunc &&callback)
{
    _jobs->PostAfter(tickAfter, std::move(callback));
}

void World::Tick()
{
    if (GetIsClosed() || _players.empty())
    {
        _isTickRunning = false;
        return;
    }

    int64 now = TimeUtils::GetTickCount64();
    float deltaTime = (now - _lastTickTime) / 1000.f;
    _lastTickTime = now;

    UpdatePosition(deltaTime);

    MulticastMove();

    PostAfter(TICK_INTERVAL_MS, [this]() { Tick(); });
}

void World::StartTick()
{
    PostAfter(TICK_INTERVAL_MS, [this]() { Tick(); });
}

void World::UpdatePosition(float deltaTime)
{

    for (auto &[id, player] : _players)
    {
        if (player->GetMoveInfo().GetNeedMove())
        {
            MoveInfo &move = player->GetMoveInfo();
            Vector3 next =
                player->GetCurrentPositionInfo().position +
                move.direction * (player->GetActorInfo().speed * deltaTime);

            // TODO : CanGo
            player->GetCurrentPositionInfo().position = next;
        }
    }
}

void World::MulticastMove()
{
    for (auto &[id, player] : _players)
    {
        MoveInfo &playerMove = player->GetMoveInfo();
        if (!playerMove.GetNeedMove() && !playerMove.lastMulticastMoving)
            continue;

        Vector3 currentPos = player->GetCurrentPositionInfo().position;

        Protocol::S_Move pkt;
        pkt.set_actor_id(player->GetActorInfo().actorId);

        Protocol::MoveData data;
        data.set_direction_x(playerMove.direction.x);
        data.set_direction_y(playerMove.direction.y);
        data.set_direction_z(playerMove.direction.z);
        data.set_yaw(playerMove.yaw);
        data.set_current_position_x(currentPos.x);
        data.set_current_position_y(currentPos.y);
        data.set_move_state(playerMove.move_state);
        *pkt.mutable_move_data() = data;
        pkt.set_server_time(_lastTickTime);

        SendBufferRef buffer = PacketHandler::MakeSendBuffer(pkt, PKT_S_Move);
        for (auto [k, other] : _players)
            other->GetSession()->Send(buffer);

        playerMove.lastMulticastMoving = playerMove.GetNeedMove();
    }
}

PlayerRef World::FindPlayer(int64 playerId)
{
    auto player = _players.find(playerId);

    return player != _players.end() ? player->second : nullptr;
}

void World::EnterPlayer(PlayerRef player)
{
    if (player == nullptr)
        return;

    if (FindPlayer(player->GetActorInfo().actorId) != nullptr)
        return;

    player->EnterWorld(GetWorld());

    GameSessionRef session = player->GetSession();
    if (session == nullptr)
        return;

    {
        Protocol::S_EnterWorld pkt;

        *pkt.mutable_selected_player_data()->mutable_player_data() =
            session->GetSelectedPlayer()->ToProto();

        SendBufferRef sendBuffer =
            PacketHandler::MakeSendBuffer(pkt, PKT_S_EnterWorld);

        session->Send(sendBuffer);
    }

    if (!_players.empty())
    {
        Protocol::S_Spawn pkt;
        for (auto &[key, otherPlayer] : _players)
        {
            pkt.mutable_players()->Add(otherPlayer->ToProto());
        }
        SendBufferRef sendBuffer =
            PacketHandler::MakeSendBuffer(pkt, PKT_S_Spawn);
        session->Send(sendBuffer);
    }

    _players.emplace(player->GetActorInfo().actorId, player);

    if (!_isTickRunning)
    {
        _isTickRunning = true;
        StartTick();
    }

    {
        Protocol::S_Spawn pkt;
        pkt.mutable_players()->Add(player->ToProto());

        SendBufferRef sendBuffer =
            PacketHandler::MakeSendBuffer(pkt, PKT_S_Spawn);

        for (auto &[key, otherPlayer] : _players)
        {
            if (otherPlayer->GetActorInfo().actorId ==
                player->GetActorInfo().actorId)
                continue;
            otherPlayer->GetSession()->Send(sendBuffer);
        }
    }
}

void World::LeavePlayer(int64 playerId)
{
    Post([this, playerId]() { LeavePlayer_Internal(playerId); });
}

void World::HandleMove(PlayerRef player, MoveInfo nextMove)
{
    Post([this, player, nextMove]() { HandleMove_Internal(player, nextMove); });
}

void World::LeavePlayer_Internal(int64 playerId)
{
    PlayerRef player = FindPlayer(playerId);
    if (player == nullptr)
        return;

    player->LeaveWorld();
    _players.erase(playerId);

    GameSessionRef session = player->GetSession();

    if (session != nullptr)
    {
        Protocol::S_LeaveWorld pkt;
        SendBufferRef sendBuffer =
            PacketHandler::MakeSendBuffer(pkt, PKT_S_LeaveWorld);
        player->GetSession()->Send(sendBuffer);
    }

    {
        Protocol::S_Despawn pkt;
        pkt.mutable_actor_ids()->Add(playerId);
        SendBufferRef sendBuffer =
            PacketHandler::MakeSendBuffer(pkt, PKT_S_Despawn);

        for (auto &[key, otherPlayer] : _players)
        {
            otherPlayer->GetSession()->Send(sendBuffer);
        }
    }
}

void World::HandleMove_Internal(PlayerRef player, MoveInfo nextMove)
{
    if (player == nullptr)
        return;

    MoveInfo &playerMove = player->GetMoveInfo();
    playerMove.direction = nextMove.direction;
    playerMove.yaw = nextMove.yaw;
    playerMove.move_state = nextMove.move_state;
}
