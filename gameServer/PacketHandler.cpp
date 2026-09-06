#include "PacketHandler.h"
#include "ActorFactory.h"
#include "GameLoop.h"
#include "GameManager.h"
#include "GameSession.h"
#include "Player.h"
#include "World.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_C_FindMyPlayers(GameSessionRef &session,
                            Protocol::C_FindMyPlayers &pkt)
{
    // TODO : db load palyers form dbId;

    auto player = ActorFactory::Create<Player>();
    player->BindStats("chulsu", 1);
    player->Bind(session);
    player->GetPositionData().y = 0.f;
    player->GetPositionData().x = 0.f;
    player->GetPositionData().yaw = 0.f;

    // TODO : split load db playerData and selectedPlayer
    session->AddPlayer(player);

    auto player2 = ActorFactory::Create<Player>();
    player2->BindStats("younghi", 5);
    player2->Bind(session);
    player2->GetPositionData().y = 10.f;
    player2->GetPositionData().x = 10.f;
    player2->GetPositionData().yaw = 10.f;

    session->AddPlayer(player2);

    auto player3 = ActorFactory::Create<Player>();
    player3->BindStats("huni", 10);
    player3->Bind(session);
    player3->GetPositionData().y = 5.f;
    player3->GetPositionData().x = 5.f;
    player3->GetPositionData().yaw = 10.f;

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
    session->ChoosePlayer(pkt.selected_player_index());

    GGameLoop.EnterWorld(0, session->GetSelectedPlayer());
    return true;
}

bool Handle_C_LeaveWorld(GameSessionRef &session, Protocol::C_LeaveWorld &pkt)
{
    if (session == nullptr || !session->IsConnected())
        return false;

    PlayerRef player = session->GetSelectedPlayer();

    if (player == nullptr)
        return false;

    WorldRef world = player->GetWorld();

    if (world == nullptr)
        return false;

    world->LeavePlayer(player->GetActorData().actorId);

    return true;
}

// std::cout << "[PacketHandler] LeaveWorld" << std::endl;
// session->Disconnect();
// return true;

bool Handle_C_Move(GameSessionRef &session, Protocol::C_Move &pkt)
{
    if (session == nullptr)
        return false;

    PlayerRef player = session->GetSelectedPlayer();

    if (player == nullptr)
        return false;

    WorldRef world = player->GetWorld();
    if (world == nullptr)
        return false;

    MoveData moveData = {
        pkt.move_data().direction_x(), pkt.move_data().direction_y(),
        pkt.move_data().direction_z(), pkt.move_data().yaw(),
        pkt.move_data().position_x(),  pkt.move_data().position_y(),
        pkt.move_data().move_state()};

    world->HandleMove(player, moveData);

    return true;
}
