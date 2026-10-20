#pragma once

#include "Types.h"
#include <chrono>
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
        std::chrono::steady_clock::time_point GetPostTime() const { return _postTime; }

    private:
        CallbackFunc _callback;
        // 잡은 Post 직전에 만들어지므로 생성 시각을 Post 시각으로 본다
        std::chrono::steady_clock::time_point _postTime = std::chrono::steady_clock::now();
};
