#include "ThreadPool.h"

#include "CoreTLS.h"
#include "Types.h"
#include <atomic>

ThreadPool::ThreadPool() { InitTLS(); }

ThreadPool::~ThreadPool() { Join(); }

void ThreadPool::Launch(std::function<void(void)> callback)
{
    std::lock_guard<std::mutex> guard(_lock);

    _threads.push_back(std::thread(
        [=]()
        {
            InitTLS();
            callback();
            DestroyTLS();
        }));
}

void ThreadPool::Join()
{
    for (std::thread &t : _threads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }
    _threads.clear();
}

void ThreadPool::InitTLS()
{
    static std::atomic<uint32> SThreadId = 1;
    LThreadId = SThreadId.fetch_add(1);
}

void ThreadPool::DestroyTLS() {}
