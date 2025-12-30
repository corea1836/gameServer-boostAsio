#include "GameLoop.h"
#include "Managers.h"
#include "Session.h"
#include "SessionFactory.h"
#include "Types.h"

void GameLoop::Broadcast(SendBufferRef sendBuffer) {
    cout << "Broadcast to " << Managers::SessionManager().SessionCounts()
         << endl;

    auto sessions = Managers::SessionManager().Sessions();
    for (auto &s : sessions) {
        if (s->IsConnected()) {
            cout << "Session " << s->GetId() << " Available Send" << endl;
            s->Send(sendBuffer);
        }
    }

    cout << "Boradcast end" << endl;
}
