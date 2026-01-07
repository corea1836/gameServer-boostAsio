#include "IoContext.h"

IoContext::IoContext()
    : _networkIoContext(),
      _gameIoContext() {
    _networkWorkGuard =
        make_unique<work_guard>(make_work_guard(_networkIoContext));
    _gameWorkguard = make_unique<work_guard>(make_work_guard(_gameIoContext));
}

void IoContext::Init() {}

void IoContext::Stop() {
    _networkWorkGuard.reset();
    _gameWorkguard.reset();
    _networkIoContext.stop();
    _gameIoContext.stop();
}

boost_io_context &IoContext::GetNetworkIoContext() { return _networkIoContext; }

boost_io_context &IoContext::GetGameIoContext() { return _gameIoContext; }
