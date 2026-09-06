#include "CoreManager.h"
#include "GameLoop.h"
#include "GameManager.h"
#include "GameSession.h"
#include "IoContext.h"
#include "PacketHandler.h"
#include "ThreadPool.h"

int main()
{
    PacketHandler::Init();
    CoreManager::Instance().Init(
        7777, []() -> SessionRef
        { return GSessionContainer.CreateSession<GameSession>(); });
    CoreManager::Instance().Start();
    GameManager::Instance();

    GGameLoop.CreateWorld();

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
