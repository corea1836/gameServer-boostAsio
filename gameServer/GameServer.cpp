
#include "GameSession.h"
#include "IoContext.h"
#include "Manager.h"
#include "PacketHandler.h"
#include "ThreadPool.h"
int main()
{
    PacketHandler::Init();
    Manager::Instance().Init(
        7777, []() -> SessionRef
        { return GSessionContainer.CreateSession<GameSession>(); });
    Manager::Instance().Start();

    for (int i = 0; i < 5; i++)
    {
        GThreadPool.Launch([i]() { GIoContext.GetNetworkIoContext().run(); });
    }

    for (int i = 0; i < 5; i++)
    {
        GThreadPool.Launch([i]() { GIoContext.GetGameIoContext().run(); });
    }

    GThreadPool.Join();
};
