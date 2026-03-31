#include "ThreadManager.h"
#include "CoreTLS.h"
#include "Types.h"

ThreadManager::ThreadManager() { InitTLS(); }

ThreadManager::~ThreadManager() { Join(); }

void ThreadManager::Launch(Function<void(void)> callback)
{
    LockGuard guard(_lock);

    _threads.push_back(Thread(
        [=]()
        {
            InitTLS();
            callback();
            DestroyTLS();
        }));
}

void ThreadManager::Join()
{
    for (Thread &t : _threads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }
    _threads.clear();
}

void ThreadManager::InitTLS()
{
    static Atomic<uint32> SThreadId = 1;
    LThreadId = SThreadId.fetch_add(1);
}

void ThreadManager::DestroyTLS() {}
