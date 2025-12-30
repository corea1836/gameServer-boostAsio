#include "SessionFactory.h"
#include "CoreMacro.h"
#include "Session.h"
#include "Types.h"

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

vector<SessionRef> SessionFactory::Sessions() {
    READ_LOCK;
    vector<SessionRef> sessions;

    for (auto const &[_, v] : _sessions) {
        sessions.push_back(v);
    }

    return sessions;
}

uint16 SessionFactory::SessionCounts() {
    READ_LOCK;
    return _sessions.size();
}
