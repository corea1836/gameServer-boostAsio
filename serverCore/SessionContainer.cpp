#include "SessionContainer.h"
#include "CoreMacro.h"
#include "ISession.h"
#include <mutex>

SessionContainer::SessionContainer() {}

SessionContainer::~SessionContainer() {}

void SessionContainer::RemoveSession(uint16 sessionId)
{
    std::lock_guard<std::mutex> guard(_lock);

    ASSERT_CRASH(_sessions.erase(sessionId));
}

void SessionContainer::AddSession(ISessionRef session)
{
    std::lock_guard<std::mutex> guard(_lock);

    _sessions.insert({session->GetId(), session});
}
