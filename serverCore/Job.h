#pragma once

#include "Types.h"
#include <functional>

using CallbackFunc = std::function<void()>;

class Job
{
    public:
        Job(CallbackFunc &&callback)
            : _callback(std::move(callback))
        {
        }

        template <typename ClassType, typename ReturnType, typename... Args>
        Job(std::shared_ptr<ClassType> owner,
            ReturnType (ClassType::*memFunc)(Args...), Args... args)
        {
            _callback = [owner, memFunc, args...]()
            { (owner.get()->*memFunc)(args...); };
        }

        void Execute() { _callback(); }

    private:
        CallbackFunc _callback;
};
