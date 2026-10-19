#pragma once

#include "BoostTypes.h"
#include "Job.h"
#include "Types.h"
#include <memory>
#include <chrono>

class JobSerializer;

// 존 틱을 절대 데드라인(start + k·interval)에 맞춰 JobSerializer 위에서 실행한다.
// 틱이 늦어 데드라인을 넘기면 밀린 틱을 몰아서 실행하지 않고 건너뛴 뒤 횟수를 센다 (ADR-003)
class FixedTicker : public std::enable_shared_from_this<FixedTicker>
{
    public:
        FixedTicker(std::shared_ptr<JobSerializer> jobs, int64 intervalMs,
                    CallbackFunc onTick);

        void Start();
        // 지금까지 건너뛴 틱 수
        int64 GetMissCount() const;

    private:
        // Functions
        void Tick();
        void AsyncWaitUntilDeadline();

        // Variables
        std::chrono::steady_clock::time_point _deadline;
        std::chrono::milliseconds _intervalMs;
        int64 _missCount = 0;
        std::shared_ptr<JobSerializer> _jobs;
        CallbackFunc _onTick;
        boost_timer _timer;
};
