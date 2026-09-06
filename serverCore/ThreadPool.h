#pragma once

#include <functional>
#include <mutex>
#include <thread>

class ThreadPool
{
    public:
        static void InitTLS();
        static void DestroyTLS();

        ThreadPool();
        ~ThreadPool();

        void Launch(std::function<void(void)> callback);
        void Join();

    private:
        std::mutex _lock;
        std::vector<std::thread> _threads;
};
