#include "PacketHandler.h"
#include "ActorFactory.h"
#include "GameLoop.h"
#include "GameManager.h"
#include "GameSession.h"
#include "Player.h"
#include "World.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX + 1];

bool Handle_C_FindMyPlayers(GameSessionRef &session,
                            Protocol::C_FindMyPlayers &pkt)
{
    // TODO : db load palyers form dbId;
    if (session->GetPlayers().size() > 0)
        return false;

    auto player = ActorFactory::Create<Player>();
    player->BindStats("chulsu", 1);
    player->Bind(session);
    player->GetCurrentPositionInfo().position.x = 0.f;
    player->GetCurrentPositionInfo().position.y = 0.f;
    player->GetCurrentPositionInfo().yaw = 0.f;
    player->GetMoveInfo().currentPosition.x = 0.f;
    player->GetMoveInfo().currentPosition.y = 0.f;

    // TODO : split load db playerData and selectedPlayer
    session->AddPlayer(player);

    auto player2 = ActorFactory::Create<Player>();
    player2->BindStats("younghi", 5);
    player2->Bind(session);
    player2->GetCurrentPositionInfo().position.x = 10.f;
    player2->GetCurrentPositionInfo().position.y = 10.f;
    player2->GetCurrentPositionInfo().yaw = 10.f;
    player2->GetMoveInfo().currentPosition.x = 10.f;
    player2->GetMoveInfo().currentPosition.y = 10.f;
    session->AddPlayer(player2);

    auto player3 = ActorFactory::Create<Player>();
    player3->BindStats("huni", 10);
    player3->Bind(session);
    player3->GetCurrentPositionInfo().position.x = 5.f;
    player3->GetCurrentPositionInfo().position.y = 5.f;
    player3->GetCurrentPositionInfo().yaw = 10.f;

    player3->GetMoveInfo().currentPosition.x = 5.f;
    player3->GetMoveInfo().currentPosition.y = 5.f;
    session->AddPlayer(player3);
    // TODO : split load db playerData and selectedPlayer

    Protocol::S_FindMyPlayers resPkt;
    resPkt.mutable_players()->Add(player->ToProto());
    resPkt.mutable_players()->Add(player2->ToProto());
    resPkt.mutable_players()->Add(player3->ToProto());

    SendBufferRef sendBuffer =
        PacketHandler::MakeSendBuffer(resPkt, PKT_S_FindMyPlayers);
    session->Send(sendBuffer);

    return true;
}

bool Handle_C_EnterWorld(GameSessionRef &session, Protocol::C_EnterWorld &pkt)
{
    if (session->GetEnteredWorld() != nullptr)
        return false;

    if (session->ChoosePlayer(pkt.selected_player_index()) == false)
        return false;

    PlayerRef player = session->GetSelectedPlayer();

    WorldRef world = GGameLoop.FindWorld(0);

    if (world == nullptr)
        return true;

    session->SetEnteredWorld(world);

    world->Post([world, player]() { world->EnterPlayer(player); });

    return true;
}

bool Handle_C_LeaveWorld(GameSessionRef &session, Protocol::C_LeaveWorld &pkt)
{
    if (session == nullptr)
        return true;

    PlayerRef player = session->GetSelectedPlayer();

    if (player == nullptr)
        return false;

    WorldRef world = session->GetEnteredWorld();

    if (world == nullptr)
        return false;

    world->LeavePlayer(player->GetActorInfo().actorId);
    session->SetEnteredWorld(nullptr);

    return true;
}

// std::cout << "[PacketHandler] LeaveWorld" << std::endl;
// session->Disconnect();
// return true;

bool Handle_C_Move(GameSessionRef &session, Protocol::C_Move &pkt)
{
    if (session == nullptr)
        return true;

    PlayerRef player = session->GetSelectedPlayer();

    if (player == nullptr)
        return false;

    WorldRef world = session->GetEnteredWorld();
    if (world == nullptr)
        return false;

    MoveInfo moveInfo = {Vector3{pkt.move_data().direction_x(),
                                 pkt.move_data().direction_y(),
                                 pkt.move_data().direction_z()},
                         pkt.move_data().yaw(),
                         Vector3{pkt.move_data().current_position_x(),
                                 pkt.move_data().current_position_y(), 0},
                         pkt.move_data().move_state(), 0};

    world->HandleMove(player, moveInfo);

    return true;
}
