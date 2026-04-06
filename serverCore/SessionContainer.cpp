#include "SessionContainer.h"
#include "CoreMacro.h"
#include "ISession.h"

SessionContainer::SessionContainer() {}

SessionContainer::~SessionContainer() {}

void SessionContainer::RemoveSession(ISessionRef session)
{
    WRITE_LOCK;
    ASSERT_CRASH(_sessions.erase(session->GetId()) != 0);
}
