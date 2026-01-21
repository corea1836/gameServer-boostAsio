#pragma once

#include "Job.h"
#include "LockQueue.h"
#include "pch.h"

class JobSerializer : public enable_shared_from_this<JobSerializer> {
    public:
        JobSerializer(boost_io_context &io_context);
        ~JobSerializer();

        void DoAsync(CallbackFunc &&callback) {
            Push(make_shared<Job>(std::move(callback)));
        }

        template <typename T, typename Ret, typename... Args>
        void DoAsync(Ret (T::*memFunc)(Args...), Args... args) {
            shared_ptr<T> owner = static_pointer_cast<T>(shared_from_this());
            Push(make_shared<Job>(owner, memFunc, std::forward<Args>(args)...));
        }

    protected:
        LockQueue<JobRef> _jobs;
        Atomic<int32> _jobCount{0};

    private:
        void Push(JobRef &&job);
        void TryExecute();
        void Execute();

    private:
        boost_io_context &_gameIoContext;
        Atomic<bool> _executing{false};
};
