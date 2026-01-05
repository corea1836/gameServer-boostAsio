#include "CoreGlobal.h"
#include "IoContext.h"
#include "Managers.h"
#include "ThreadManager.h"
#include "pch.h"

int main() {
    Managers::Instance().Init(7777);

    for (int i = 0; i < 5; i++) {
        GThreadManager->Launch([i]() {
            Managers::Instance().IoContextManager().GetNetworkIoContext().run();
        });
    }

    for (int i = 0; i < 5; i++) {
        GThreadManager->Launch([i]() {
            Managers::Instance().IoContextManager().GetGameIoContext().run();
        });
    }

    GThreadManager->Join();
};
