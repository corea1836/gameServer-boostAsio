#pragma once

#include "Types.h"
#include <thread>
#include <vector>
class ThreadManager
{
    public:
        ThreadManager();
        ~ThreadManager();

    public:
        void Launch(Function<void(void)> callback);
        void Join();

        static void InitTLS();
        static void DestroyTLS();

    private:
        Mutex _lock;
        Vector<Thread> _threads;
};
