#pragma once

#include "BoostTypes.h"
#include "Job.h"
#include "Types.h"
#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <queue>
#include <deque>
#include <vector>

class JobSerializer : public std::enable_shared_from_this<JobSerializer>
{
    public:
        enum
        {
            DRAIN_COUNT = 100,
            EXECUTE_MAX_DURATION_MS = 64,
            // 샘플을 꺼내 가지 않는 큐(GameLoop)가 끝없이 쌓지 않도록 둔 상한
            MAX_WAIT_SAMPLES = 65536,
        };

        JobSerializer(boost_io_context &gameIoContext);
        ~JobSerializer() = default;

        void Post(CallbackFunc &&callback)
        {
            if (GetIsClosed())
                return;
            AddJob(std::make_shared<Job>(std::move(callback)));
        }

        template <typename ClassType, typename ReturnType, typename... Args>
        void Post(std::shared_ptr<ClassType> owner,
                  ReturnType (ClassType::*memFunc)(Args...), Args... args)
        {
            if (GetIsClosed())
                return;
            AddJob(std::make_shared<Job>(owner, memFunc, args...));
        }

        void PostAfter(int64 tickAfter, CallbackFunc &&callback)
        {
            auto timer = std::make_shared<boost_timer>(_gameIoContext);
            timer->expires_after(std::chrono::milliseconds(tickAfter));

            timer->async_wait(
                [weakSelf = weak_from_this(), timer,
                 callback =
                     std::move(callback)](boost_error_code errorCode) mutable
                {
                    if (errorCode)
                        return;

                    auto self = weakSelf.lock();
                    if (self == nullptr)
                        return;

                    self->Post(std::move(callback));
                });
        }

        bool GetIsClosed() { return _isClosed.load(); }
        void Close() { _isClosed.store(true); }
        boost_io_context &GetGameIoContext() { return _gameIoContext; }

        // 잡이 Post된 뒤 실행을 시작하기까지 기다린 시간(us)을 꺼낸다(1-8 지표).
        // Execute와 같은 스레드여야 하므로 이 JobSerializer의 잡 안에서 부른다
        std::vector<int64> TakeWaitSamplesUs()
        {
            std::vector<int64> samples;
            samples.swap(_waitSamplesUs);
            return samples;
        }

    private:
        void AddJob(JobRef job);
        void Execute();

        std::queue<JobRef> _jobs;
        std::deque<JobRef> _executeJobs;
        std::vector<int64> _waitSamplesUs;
        boost_io_context &_gameIoContext;
        std::mutex _lock;
        std::atomic<bool> _isExecuting = false;
        std::atomic<bool> _isClosed = false;
};
