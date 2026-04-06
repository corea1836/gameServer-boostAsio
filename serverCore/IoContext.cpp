#include "IoContext.h"

IoContext::IoContext()
    : _networkIoContext(),
      _networkWorkGuard(_networkIoContext.get_executor()),
      _gameIoContext(),
      _gameWorkGuard(_gameIoContext.get_executor())
{
}

void IoContext::Stop()
{
    _networkWorkGuard.reset();
    _gameWorkGuard.reset();
    _networkIoContext.stop();
    _gameIoContext.stop();
}
