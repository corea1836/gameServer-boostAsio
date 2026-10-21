#include <gtest/gtest.h>

#include "GameLoop.h"
#include "GameManager.h"
#include "GameSession.h"
#include "PacketHandler.h"
#include <CoreManager.h>
#include <SendBuffer.h>

// 1-5 상태 규칙. 세션 strand는 클라가 보낸 순서 그대로 패킷을 보므로, 상태에 맞지 않는 패킷은 위반(false → 끊기)이다
//   C_FindMyPlayers: 접속 직후(목록 없음) → 목록 받음
//   C_EnterWorld:    목록 받음 + 올바른 index → 월드 안
//   C_Move:          월드 안
//   C_LeaveWorld:    월드 안 → 목록 받음
// 테스트는 io_context를 돌리지 않는다. World 큐에 넣은 입장/퇴장은 실행되지 않고, 핸들러의 반환값만 본다
namespace
{
    template <typename T>
    bool Handle(GameSessionRef &session, T &pkt, uint16 id)
    {
        SendBufferRef buffer = PacketHandler::MakeSendBuffer(pkt, id);
        return PacketHandler::HandlePacket(session, buffer->GetData(), buffer->GetWriteSize());
    }

    bool FindMyPlayers(GameSessionRef &session)
    {
        Protocol::C_FindMyPlayers pkt;
        return Handle(session, pkt, PKT_C_FindMyPlayers);
    }

    bool EnterWorld(GameSessionRef &session, int32 index)
    {
        Protocol::C_EnterWorld pkt;
        pkt.set_selected_player_index(index);
        return Handle(session, pkt, PKT_C_EnterWorld);
    }

    bool Move(GameSessionRef &session)
    {
        Protocol::C_Move pkt;
        return Handle(session, pkt, PKT_C_Move);
    }

    bool LeaveWorld(GameSessionRef &session)
    {
        Protocol::C_LeaveWorld pkt;
        return Handle(session, pkt, PKT_C_LeaveWorld);
    }
}

class SessionState : public ::testing::Test
{
    protected:
        static void SetUpTestSuite()
        {
            CoreManager::Instance().Init();
            PacketHandler::Init();
            GGameLoop.CreateWorld();
        }

        GameSessionRef _session = std::make_shared<GameSession>(1);
};

TEST_F(SessionState, NormalFlowIsAccepted)
{
    EXPECT_TRUE(FindMyPlayers(_session));
    EXPECT_TRUE(EnterWorld(_session, 0));
    EXPECT_TRUE(Move(_session));
    EXPECT_TRUE(LeaveWorld(_session));
}

TEST_F(SessionState, ReenterAfterLeaveIsAccepted)
{
    ASSERT_TRUE(FindMyPlayers(_session));
    ASSERT_TRUE(EnterWorld(_session, 0));
    ASSERT_TRUE(LeaveWorld(_session));

    EXPECT_TRUE(EnterWorld(_session, 1));
    EXPECT_TRUE(Move(_session));
}

TEST_F(SessionState, FindMyPlayersTwiceIsViolation)
{
    ASSERT_TRUE(FindMyPlayers(_session));

    EXPECT_FALSE(FindMyPlayers(_session));
}

TEST_F(SessionState, FindMyPlayersInWorldIsViolation)
{
    ASSERT_TRUE(FindMyPlayers(_session));
    ASSERT_TRUE(EnterWorld(_session, 0));

    EXPECT_FALSE(FindMyPlayers(_session));
}

TEST_F(SessionState, EnterBeforeFindMyPlayersIsViolation)
{
    EXPECT_FALSE(EnterWorld(_session, 0));
}

TEST_F(SessionState, EnterWithInvalidIndexIsViolation)
{
    ASSERT_TRUE(FindMyPlayers(_session));

    EXPECT_FALSE(EnterWorld(_session, 3));
    EXPECT_FALSE(EnterWorld(_session, -1));
}

TEST_F(SessionState, EnterTwiceIsViolation)
{
    ASSERT_TRUE(FindMyPlayers(_session));
    ASSERT_TRUE(EnterWorld(_session, 0));

    EXPECT_FALSE(EnterWorld(_session, 0));
}

TEST_F(SessionState, MoveBeforeEnterIsViolation)
{
    EXPECT_FALSE(Move(_session));

    ASSERT_TRUE(FindMyPlayers(_session));
    EXPECT_FALSE(Move(_session));
}

TEST_F(SessionState, MoveAfterLeaveIsViolation)
{
    ASSERT_TRUE(FindMyPlayers(_session));
    ASSERT_TRUE(EnterWorld(_session, 0));
    ASSERT_TRUE(LeaveWorld(_session));

    EXPECT_FALSE(Move(_session));
}

TEST_F(SessionState, LeaveBeforeEnterIsViolation)
{
    EXPECT_FALSE(LeaveWorld(_session));

    ASSERT_TRUE(FindMyPlayers(_session));
    EXPECT_FALSE(LeaveWorld(_session));
}
