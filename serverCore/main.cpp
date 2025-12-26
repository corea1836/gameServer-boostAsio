#include "CoreTLS.h"
#include "ThreadManager.h"
#include "pch.h"
#include <chrono>
#include <iostream>
#include <thread>

void ThreadMain() {
    while (true) {
        cout << "Hello World! I am thread[" << LThreadId << "]" << endl;
        this_thread::sleep_for(chrono::seconds(3));
    }
}

int main() {
    ThreadManager *tm = new ThreadManager();
    for (int32 i = 0; i < 5; i++) {
        tm->Launch(ThreadMain);
    }

    tm->Join();
}
