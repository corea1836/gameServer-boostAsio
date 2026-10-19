#pragma once

#include "BoostTypes.h"
#include "Job.h"
#include "Types.h"
#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <deque>

class JobSerializer : public std::enable_shared_from_this<JobSerializer>
{
    public:
        enum
        {
            DRAIN_COUNT = 100,
            EXECUTE_MAX_DURATION_MS = 64,
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

        template <typename ClassType, typename ReturnType, typename... Args>
        void PostAfter(int64 tickAfter, std::shared_ptr<ClassType> owner,
                       ReturnType (ClassType::*memFunc)(Args...), Args... args)
        {
            auto timer = std::make_shared<boost_timer>(_gameIoContext);
            timer->expires_after(std::chrono::milliseconds(tickAfter));

            timer->async_wait(
                [weakSelf = weak_from_this(), timer, owner, memFunc,
                 args...](boost_error_code errorCode) mutable
                {
                    if (errorCode)
                        return;

                    auto self = weakSelf.lock();
                    if (self == nullptr)
                        return;

                    self->Post(std::make_shared<Job>(owner, memFunc, args...));
                });
        }

        bool GetIsClosed() { return _isClosed.load(); }
        void Close() { _isClosed.store(true); }

    private:
        void AddJob(JobRef job);
        void Execute();

        std::queue<JobRef> _jobs;
        std::deque<JobRef> _executeJobs;
        boost_io_context &_gameIoContext;
        std::mutex _lock;
        std::atomic<bool> _isExecuting = false;
        std::atomic<bool> _isClosed = false;
};
