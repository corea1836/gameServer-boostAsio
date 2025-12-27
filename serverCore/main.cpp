#include "CoreMacro.h"
#include "Lock.h"
#include "ThreadManager.h"
#include "pch.h"
#include <cstdlib>
#include <queue>
#include <thread>

class TestLock {
        USE_LOCK;

    public:
        int32 TestRead() {
            READ_LOCK;

            if (_queue.empty())
                return -1;

            return _queue.front();
        }

        void TestPush() {
            WRITE_LOCK;

            _queue.push(rand() % 100);
        }

        void TestPop() {
            WRITE_LOCK;

            if (_queue.empty() == false)
                _queue.pop();
        }

    private:
        queue<int32> _queue;
};

TestLock testLock;

void ThreadWrite() {
    while (true) {
        testLock.TestPush();
        this_thread::sleep_for(std::chrono::seconds(1));
        testLock.TestPop();
    }
}

void ThreadRead() {
    while (true) {
        int32 value = testLock.TestRead();
        cout << value << endl;
        this_thread::sleep_for(std::chrono::seconds(1));
    }
}

ThreadManager threadManager;

int main() {
    for (int32 i = 0; i < 2; i++) {
        threadManager.Launch(ThreadWrite);
    }

    for (int32 i = 0; i < 5; i++) {
        threadManager.Launch(ThreadRead);
    }

    threadManager.Join();
}
