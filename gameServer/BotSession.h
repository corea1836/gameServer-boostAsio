#pragma once

#include <BoostTypes.h>
#include <Session.h>
#include <Types.h>
#include <atomic>
#include <random>

class BotSession;
using BotSessionRef = std::shared_ptr<BotSession>;

// 봇 전체 진행 상황. 여러 네트워크 스레드가 더한다
struct BotStats
{
        static inline std::atomic<int32> SConnected = 0;
        static inline std::atomic<int32> SEntered = 0;
        static inline std::atomic<int32> SDisconnected = 0;
        // 받은 S_Spawn/S_Despawn에 담긴 액터 수 (AOI 가시성 변화량)
        static inline std::atomic<int64> SSpawned = 0;
        static inline std::atomic<int64> SDespawned = 0;
};

// 부하 테스트 봇(1-9). 접속 → C_FindMyPlayers → C_EnterWorld(0) → 흩어지기 → 0.5~2초마다 8방향 이동/정지.
// 실제 클라(SelectedPlayer.cs)처럼 입력이 바뀌면 C_Move를 보내고, 움직이는 동안은 250ms마다 다시 보낸다.
// 수신 처리와 이동 타이머는 서로 다른 스레드에서 돌 수 있어서, 이동 상태는 타이머 콜백에서만 만진다
class BotSession : public Session
{
    public:
        BotSession(int64 sessionId);

        BotSessionRef GetBotSession()
        {
            return std::static_pointer_cast<BotSession>(shared_from_this());
        }

        void OnFindMyPlayers();
        void OnEnterWorld();

    protected:
        bool OnRead(byte *readPos, int32 readSize) override;
        void OnConnected() override;
        void OnDisconnected() override;

    private:
        void AsyncWaitMove();
        void UpdateMove();
        void SendMove();

        boost_timer _moveTimer;
        std::mt19937 _random;
        int32 _decisionLeftMs = 0;
        // 0~7: 8방향, -1: 정지
        int32 _direction = -1;
        // 입장 직후 이 각도로 걷는 남은 시간. 0이 되면 한 번 멈추고 랜덤 이동으로 넘어간다
        int32 _scatterLeftMs = 0;
        float _scatterAngle = 0;
};
