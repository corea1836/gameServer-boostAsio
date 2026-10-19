#include <gtest/gtest.h>

#include "JobSerializer.h"
#include <chrono>
#include <numeric>
#include <vector>

namespace
{

// sleep은 요청보다 오래 잘 수 있어서, ms만큼 바쁘게 기다린다
void BusyWait(int64 ms)
{
    auto end =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(ms);
    while (std::chrono::steady_clock::now() < end)
    {
    }
}

constexpr int HEAVY_JOB_COUNT = 100;

} // namespace

// io_context를 이 스레드 하나로만 돌려서 게임 스레드 1개를 흉내 낸다.
// 무거운 존이 시간 예산을 넘기면 양보해서, 가벼운 존이 그 사이에 실행돼야 한다
TEST(JobSerializer, HeavyQueueYieldsAfterBudget)
{
    boost_io_context ioContext;
    auto heavy = std::make_shared<JobSerializer>(ioContext);
    auto light = std::make_shared<JobSerializer>(ioContext);

    int heavyDone = 0;
    int heavyDoneWhenLightRan = -1;

    for (int i = 0; i < HEAVY_JOB_COUNT; i++)
        heavy->Post(
            [&heavyDone]()
            {
                BusyWait(1);
                heavyDone++;
            });
    light->Post([&]() { heavyDoneWhenLightRan = heavyDone; });

    ioContext.run();

    EXPECT_EQ(heavyDone, HEAVY_JOB_COUNT);
    // 1ms 잡을 잡 단위로 예산 확인하면, 예산(ms)만큼 + 경계 오차 정도만 실행하고 양보한다
    EXPECT_LE(heavyDoneWhenLightRan, JobSerializer::EXECUTE_MAX_DURATION_MS + 2);
}

// 양보하느라 배치 중간에 멈췄다가 이어서 실행해도, 잡이 빠지거나 순서가 바뀌면 안 된다.
// 실행 도중에 새로 들어온 잡(100)은 이미 꺼내 둔 잡들보다 뒤에 실행돼야 한다
TEST(JobSerializer, YieldKeepsOrderAndLosesNoJob)
{
    boost_io_context ioContext;
    auto heavy = std::make_shared<JobSerializer>(ioContext);
    std::vector<int> order;

    for (int i = 0; i < HEAVY_JOB_COUNT; i++)
        heavy->Post(
            [&, i]()
            {
                BusyWait(1);
                order.push_back(i);
                if (i == 10)
                    heavy->Post([&]()
                                { order.push_back(HEAVY_JOB_COUNT); });
            });

    ioContext.run();

    std::vector<int> expected(HEAVY_JOB_COUNT + 1);
    std::iota(expected.begin(), expected.end(), 0);
    EXPECT_EQ(order, expected);
}
