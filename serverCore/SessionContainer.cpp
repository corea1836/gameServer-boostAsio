#include "SessionContainer.h"
#include "CoreMacro.h"
#include "Session.h"
#include <mutex>

SessionContainer::SessionContainer() {}

SessionContainer::~SessionContainer() {}

void SessionContainer::RemoveSession(int64 sessionId)
{
    std::lock_guard<std::mutex> guard(_lock);

    ASSERT_CRASH(_sessions.erase(sessionId));
}

void SessionContainer::AddSession(SessionRef session)
{
    std::lock_guard<std::mutex> guard(_lock);

    _sessions.insert({session->GetId(), session});
}
