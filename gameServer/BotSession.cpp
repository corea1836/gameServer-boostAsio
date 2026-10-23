#include "BotSession.h"
#include "BotPacketHandler.h"
#include "CoreManager.h"
#include "IoContext.h"
#include "PacketHandler.h"
#include <cmath>

namespace
{

// 클라가 움직이는 동안 C_Move를 다시 보내는 주기 (SelectedPlayer.cs의 _sendInterval)
constexpr int32 RESEND_INTERVAL_MS = 250;
constexpr int32 MIN_DECISION_MS = 500;
constexpr int32 MAX_DECISION_MS = 2000;
constexpr float PI = 3.14159265f;
// 모두 (0,0)에서 스폰해서, AOI 효과를 재려면 가시 반경보다 넓게 흩어져야 한다(ADR-022).
// 걷는 시간 = 최대 × sqrt(균등 난수)라서 반경 약 50(속도 5 × 10초) 원 안에 고르게 퍼진다
constexpr int32 SCATTER_MAX_MS = 10000;

} // namespace

BotSession::BotSession(int64 sessionId)
    : Session(sessionId),
      _moveTimer(GIoContext.GetNetworkIoContext()),
      _random(static_cast<uint32>(sessionId))
{
    std::uniform_real_distribution<float> unit(0.f, 1.f);
    _scatterLeftMs = static_cast<int32>(SCATTER_MAX_MS * std::sqrt(unit(_random)));
    _scatterAngle = unit(_random) * 2 * PI;
}

bool BotSession::OnRead(byte *readPos, int32 readSize)
{
    BotSessionRef session = GetBotSession();
    return BotPacketHandler::HandlePacket(session, readPos, readSize);
}

void BotSession::OnConnected()
{
    BotStats::SConnected.fetch_add(1);

    Protocol::C_FindMyPlayers pkt;
    Send(PacketHandler::MakeSendBuffer(pkt, PKT_C_FindMyPlayers));
}

void BotSession::OnDisconnected() { BotStats::SDisconnected.fetch_add(1); }

void BotSession::OnFindMyPlayers()
{
    Protocol::C_EnterWorld pkt;
    pkt.set_selected_player_index(0);
    Send(PacketHandler::MakeSendBuffer(pkt, PKT_C_EnterWorld));
}

// 이동 타이머는 여기서 한 번만 시작하고, 그 뒤로는 타이머 콜백만 이동 상태를 만진다
void BotSession::OnEnterWorld()
{
    BotStats::SEntered.fetch_add(1);
    AsyncWaitMove();
}

void BotSession::AsyncWaitMove()
{
    _moveTimer.expires_after(std::chrono::milliseconds(RESEND_INTERVAL_MS));
    _moveTimer.async_wait(
        [self = GetBotSession()](boost_error_code errorCode)
        {
            if (errorCode || !self->IsConnected())
                return;

            self->UpdateMove();
            self->AsyncWaitMove();
        });
}

void BotSession::UpdateMove()
{
    if (_scatterLeftMs > 0)
    {
        _scatterLeftMs -= RESEND_INTERVAL_MS;
        // 다 걸었으면 _direction(-1)대로 멈춤을 보낸다
        SendMove();
        return;
    }

    _decisionLeftMs -= RESEND_INTERVAL_MS;
    if (_decisionLeftMs <= 0)
    {
        _decisionLeftMs = std::uniform_int_distribution<int32>(
            MIN_DECISION_MS, MAX_DECISION_MS)(_random);

        int32 next = std::uniform_int_distribution<int32>(-1, 7)(_random);
        if (next != _direction)
        {
            _direction = next;
            SendMove();
            return;
        }
    }

    if (_direction >= 0)
        SendMove();
}

void BotSession::SendMove()
{
    Protocol::C_Move pkt;
    Protocol::MoveData *move = pkt.mutable_move_data();

    bool scattering = _scatterLeftMs > 0;
    if (_direction < 0 && !scattering)
    {
        move->set_move_state(Protocol::EMOVE_STATE_STOP);
    }
    else
    {
        // 8방향을 45도 간격으로 고른다(흩어지는 동안은 임의의 각도). 대각선도 길이 1(정규화된 방향)
        float angle = scattering ? _scatterAngle : _direction * (PI / 4);
        float x = std::sin(angle);
        float y = std::cos(angle);
        move->set_direction_x(x);
        move->set_direction_y(y);
        // 클라와 같은 규약: yaw = atan2(x, y) (도)
        move->set_yaw(std::atan2(x, y) * 180.f / PI);
        move->set_move_state(Protocol::EMOVE_STATE_MOVE);
    }

    Send(PacketHandler::MakeSendBuffer(pkt, PKT_C_Move));
}
