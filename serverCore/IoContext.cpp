#include "IoContext.h"
#include "Types.h"
#include <memory>

IoContext::IoContext()
    : _networkIoContext(),
      _gameIoContext() {
    _networkWorkGuard =
        make_unique<work_guard>(make_work_guard(_networkIoContext));
    _gameWorkguard = make_unique<work_guard>(make_work_guard(_gameIoContext));

    cout << "IoContext created with work_guards" << endl;
}

void IoContext::Init() {}

void IoContext::Stop() {
    cout << "Stopping IoContext..." << endl;
    _networkWorkGuard.reset();
    _gameWorkguard.reset();
    _networkIoContext.stop();
    _gameIoContext.stop();
}

io_context &IoContext::GetNetworkIoContext() { return _networkIoContext; }

io_context &IoContext::GetGameIoContext() { return _gameIoContext; }
