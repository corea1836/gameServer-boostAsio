#include "World.h"
#include "CoreManager.h"
#include "GameSession.h"
#include "IoContext.h"
#include "PacketHandler.h"
#include "Player.h"
#include <JobSerializer.h>

World::World() { _jobs = std::make_unique<JobSerializer>(GIoContext.GetGameIoContext()); }

World::~World() {}

void World::Post(CallbackFunc &&callback) { _jobs->Post(std::move(callback)); }

void World::PostAfter(int64 tickAfter, CallbackFunc &&callback)
{
    _jobs->PostAfter(tickAfter, std::move(callback));
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

    if (FindPlayer(player->GetActorData().actorId) != nullptr)
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

    _players.emplace(player->GetActorData().actorId, player);

    {
        Protocol::S_Spawn pkt;
        pkt.mutable_players()->Add(player->ToProto());

        SendBufferRef sendBuffer =
            PacketHandler::MakeSendBuffer(pkt, PKT_S_Spawn);

        for (auto &[key, otherPlayer] : _players)
        {
            if (otherPlayer->GetActorData().actorId ==
                player->GetActorData().actorId)
                continue;
            otherPlayer->GetSession()->Send(sendBuffer);
        }
    }
}

void World::LeavePlayer(int64 playerId)
{
    Post([this, playerId]() { LeavePlayer_Internal(playerId); });
}

void World::HandleMove(PlayerRef player, MoveData moveData)
{
    Post([this, player, moveData]() { HandleMove_Internal(player, moveData); });
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

void World::HandleMove_Internal(PlayerRef player, MoveData moveData)
{
    if (player == nullptr)
        return;

    player->GetMoveData().direction_x = moveData.direction_x;
    player->GetMoveData().direction_y = moveData.direction_y;
    player->GetMoveData().direction_z = moveData.direction_z;
    player->GetMoveData().yaw = moveData.yaw;
    player->GetMoveData().position_x = moveData.position_x;
    player->GetMoveData().position_y = moveData.position_y;
    player->GetMoveData().move_state = moveData.move_state;

    //   player->GetPositionData().x = moveData.position_x;
    //  player->GetPositionData().y = moveData.position_y;
    //  player->GetPositionData().y = moveData.yaw;

    {
        Protocol::S_Move pkt;

        pkt.set_actor_id(player->GetActorData().actorId);
        *pkt.mutable_move_data() = player->GetMoveData().ToProto();

        SendBufferRef sendBuffer =
            PacketHandler::MakeSendBuffer(pkt, PKT_S_Move);

        for (auto &[key, otherPlayer] : _players)
        {
            if (otherPlayer->GetActorData().actorId ==
                player->GetActorData().actorId)
                continue;
            otherPlayer->GetSession()->Send(sendBuffer);
        }
    }
}
