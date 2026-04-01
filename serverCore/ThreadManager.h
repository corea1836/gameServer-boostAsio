#pragma once

#include "Types.h"

class ThreadManager
{
    public:
        ThreadManager();
        ~ThreadManager();

    public:
        void Launch(function<void(void)> callback);
        void Join();

        static void InitTLS();
        static void DestroyTLS();

    private:
        mutex _lock;
        vector<thread> _threads;
};
