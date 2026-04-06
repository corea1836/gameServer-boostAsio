#pragma once

#include <functional>
#include <mutex>
#include <thread>

class ThreadPool
{
    public:
        ThreadPool();
        ~ThreadPool();

    public:
        void Launch(std::function<void(void)> callback);
        void Join();

        static void InitTLS();
        static void DestroyTLS();

    private:
        std::mutex _lock;
        std::vector<std::thread> _threads;
};
