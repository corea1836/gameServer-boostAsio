#include "SessionFactory.h"
#include "Session.h"

SessionFactory::SessionFactory() {}

SessionFactory::~SessionFactory() {}

SessionRef SessionFactory::CreateSession() {
    WRITE_LOCK;

    uint16 sessionId = ++_sessionId;

    SessionRef session = make_shared<Session>(sessionId);
    _sessions.insert({sessionId, session});

    cout << "Session " << sessionId << " Created." << endl;

    return session;
}

void SessionFactory::RemoveSession(SessionRef session) {
    WRITE_LOCK;
    ASSERT_CRASH(_sessions.erase(session->GetId()) != 0);
}
