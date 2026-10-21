#include "World.h"
#include "Actor.h"
#include "CoreManager.h"
#include "FixedTicker.h"
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
#include <chrono>

World::World(int64 worldId)
    : _worldId(worldId)
{
}

World::~World() {}

void World::Init()
{
    _jobs = std::make_shared<JobSerializer>(GIoContext.GetGameIoContext());
    _fixedTicker = std::make_shared<FixedTicker>(_jobs, TICK_INTERVAL_MS, [this, weakSelf = weak_from_this()]()
    {
        auto self = weakSelf.lock();
        if (self == nullptr)
            return;

        Tick();
    });

    _fixedTicker->Start();
}

void World::Post(CallbackFunc &&callback) { _jobs->Post(std::move(callback)); }

void World::Tick()
{
    auto tickStart = std::chrono::steady_clock::now();
    auto now = TimeUtils::GetTickCount64();

    UpdatePosition(TICK_INTERVAL_MS / 1000.f);

    MulticastMove(now);

    _metrics.RecordTick(std::chrono::steady_clock::now() - tickStart);
    _metrics.ReportIfDue(_worldId, static_cast<int64>(_players.size()), _fixedTicker->GetMissCount(), *_jobs);
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

void World::MulticastMove(int64 now)
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
        pkt.set_server_time(now);

        SendBufferRef buffer = PacketHandler::MakeSendBuffer(pkt, PKT_S_Move);
        for (auto [k, other] : _players)
        {
            GameSessionRef otherSession = other->GetSession();
            if (otherSession == nullptr)
                continue;

            otherSession->Send(buffer);
        }


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

    {
        Protocol::S_Spawn pkt;
        pkt.mutable_players()->Add(player->ToProto());

        SendBufferRef sendBuffer =
            PacketHandler::MakeSendBuffer(pkt, PKT_S_Spawn);

        for (auto &[key, otherPlayer] : _players)
        {
            GameSessionRef otherSession = otherPlayer->GetSession();
            if (otherSession == nullptr || otherPlayer->GetActorInfo().actorId == player->GetActorInfo().actorId)
                continue;
            otherSession->Send(sendBuffer);
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

    GameSessionRef session = player->GetSession();

    player->LeaveWorld();
    _players.erase(playerId);

    if (session != nullptr)
    {
        Protocol::S_LeaveWorld pkt;
        SendBufferRef sendBuffer =
            PacketHandler::MakeSendBuffer(pkt, PKT_S_LeaveWorld);
        session->Send(sendBuffer);
    }

    {
        Protocol::S_Despawn pkt;
        pkt.mutable_actor_ids()->Add(playerId);
        SendBufferRef sendBuffer =
            PacketHandler::MakeSendBuffer(pkt, PKT_S_Despawn);

        for (auto &[key, otherPlayer] : _players)
        {
            GameSessionRef otherSession = otherPlayer->GetSession();
            if (otherSession == nullptr)
                continue;
            otherSession->Send(sendBuffer);
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
