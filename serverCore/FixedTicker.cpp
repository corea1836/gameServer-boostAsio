#include "FixedTicker.h"

#include <JobSerializer.h>

FixedTicker::FixedTicker(std::shared_ptr<JobSerializer> jobs, int64 intervalMs, CallbackFunc onTick)
                             : _intervalMs(intervalMs), _jobs(jobs), _onTick(onTick), _timer(jobs->GetGameIoContext())
{
}

void FixedTicker::Start()
{
    _deadline = std::chrono::steady_clock::now() + _intervalMs;
    AsyncWaitUntilDeadline();
}

int64 FixedTicker::GetMissCount() const { return _missCount; }

void FixedTicker::Tick() {
    _onTick();

    _deadline += _intervalMs;
    auto now = std::chrono::steady_clock::now();

    while (_deadline < now)
    {
        _missCount++;
        _deadline +=_intervalMs;
    }

    AsyncWaitUntilDeadline();
}

void FixedTicker::AsyncWaitUntilDeadline()
{
    _timer.expires_at(_deadline);

    _timer.async_wait([this, weakSelf = weak_from_this()](boost::system::error_code errorCode) mutable
    {
        if (errorCode)
            return;

        auto self = weakSelf.lock();
        if (self == nullptr)
            return;

        _jobs->Post([this, weakSelf]()
        {
            auto self = weakSelf.lock();
            if (self == nullptr)
                return;

            Tick();
        });
    });
}
