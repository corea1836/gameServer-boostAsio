#include <gtest/gtest.h>

#include "FixedTicker.h"
#include "JobSerializer.h"
#include <chrono>
#include <vector>

namespace
{

using Clock = std::chrono::steady_clock;

// sleep은 요청보다 오래 잘 수 있어서, ms만큼 바쁘게 기다린다
void BusyWait(int64 ms)
{
    auto end = Clock::now() + std::chrono::milliseconds(ms);
    while (Clock::now() < end)
    {
    }
}

constexpr int64 TICK_INTERVAL_MS = 50; // 20Hz (ADR-003)
constexpr auto RUN_DURATION = std::chrono::milliseconds(1000);
constexpr int TICKS_PER_RUN = 20;

} // namespace

// io_context를 이 스레드 하나로만 1초 돌린다. 20Hz면 틱이 20번 안팎 실행된다
TEST(FixedTicker, TicksTwentyTimesPerSecond)
{
    boost_io_context ioContext;
    auto jobs = std::make_shared<JobSerializer>(ioContext);
    int tickCount = 0;
    auto ticker = std::make_shared<FixedTicker>(
        jobs, TICK_INTERVAL_MS, [&tickCount]() { tickCount++; });

    ticker->Start();
    ioContext.run_for(RUN_DURATION);

    EXPECT_NEAR(tickCount, TICKS_PER_RUN, 1);
    EXPECT_EQ(ticker->GetMissCount(), 0);
}

// 틱마다 10ms씩 일을 해도 다음 틱은 "끝난 시점 + 50ms"가 아니라 "start + k·50ms"에 온다.
// 상대 시간으로 예약하면 60ms 주기가 돼서 1초에 16~17번밖에 돌지 않는다
TEST(FixedTicker, TickCostDoesNotDelayNextDeadline)
{
    constexpr int64 TICK_COST_MS = 10;
    boost_io_context ioContext;
    auto jobs = std::make_shared<JobSerializer>(ioContext);
    int tickCount = 0;
    auto ticker = std::make_shared<FixedTicker>(jobs, TICK_INTERVAL_MS,
                                                [&tickCount]()
                                                {
                                                    BusyWait(TICK_COST_MS);
                                                    tickCount++;
                                                });

    ticker->Start();
    ioContext.run_for(RUN_DURATION);

    EXPECT_NEAR(tickCount, TICKS_PER_RUN, 1);
    EXPECT_EQ(ticker->GetMissCount(), 0);
}

// 한 틱이 120ms 걸리면 그동안 지나간 데드라인 2개(+50, +100)는 건너뛰고 +150에 다음 틱을 돈다.
// 밀린 틱을 몰아서 실행하면 느린 틱 직후의 틱들이 간격 없이 붙어서 실행된다
TEST(FixedTicker, SlowTickSkipsMissedDeadlinesWithoutCatchUp)
{
    constexpr size_t SLOW_TICK_INDEX = 5;
    constexpr int64 SLOW_TICK_MS = 120;
    boost_io_context ioContext;
    auto jobs = std::make_shared<JobSerializer>(ioContext);
    std::vector<Clock::time_point> tickStarts;
    auto ticker = std::make_shared<FixedTicker>(
        jobs, TICK_INTERVAL_MS,
        [&tickStarts]()
        {
            tickStarts.push_back(Clock::now());
            if (tickStarts.size() == SLOW_TICK_INDEX)
                BusyWait(SLOW_TICK_MS);
        });

    ticker->Start();
    ioContext.run_for(RUN_DURATION);

    EXPECT_EQ(ticker->GetMissCount(), 2);
    EXPECT_NEAR(static_cast<int>(tickStarts.size()), TICKS_PER_RUN - 2, 1);
    for (size_t i = 1; i < tickStarts.size(); i++)
    {
        auto gapMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                         tickStarts[i] - tickStarts[i - 1])
                         .count();
        EXPECT_GE(gapMs, TICK_INTERVAL_MS / 2)
            << i << "번째 틱이 직전 틱에 붙어서 실행됐다 (몰아서 실행)";
    }
}
