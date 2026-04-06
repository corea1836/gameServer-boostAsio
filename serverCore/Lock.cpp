#include "Lock.h"
#include "CoreMacro.h"
#include "CoreTLS.h"
#include "TimeUtils.h"
#include <thread>

void Lock::WriteLock()
{
    const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
    if (LThreadId == lockThreadId)
    {
        _writeCount++;
        return;
    }

    const uint64 beginTick = TimeUtils::GetTickCount64();
    const uint32 desired = ((LThreadId << 16) & WRITE_THREAD_MASK);

    while (true)
    {
        for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
        {
            uint32 expected = EMPTY_FLAG;
            if (_lockFlag.compare_exchange_strong(expected, desired))
            {
                _writeCount++;
                return;
            }
        }

        if (TimeUtils::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
            CRASH("LOCK_TIMEOUT");

        std::this_thread::yield();
    }
}

void Lock::WriteUnlock()
{
    if ((_lockFlag.load() & READ_COUNT_MASK) != 0)
        CRASH("INVALID_UNLOCK_ORDER");

    const int32 lockCount = --_writeCount;
    if (lockCount == 0)
        _lockFlag.store(EMPTY_FLAG);
}

void Lock::ReadLock()
{
    const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
    if (LThreadId == lockThreadId)
    {
        _lockFlag.fetch_add(1);
        return;
    }

    const uint64 beginTick = TimeUtils::GetTickCount64();
    while (true)
    {
        for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
        {
            uint32 expected = (_lockFlag.load() & READ_COUNT_MASK);
            if (_lockFlag.compare_exchange_strong(expected, expected + 1))
                return;
        }

        if (TimeUtils::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
            CRASH("LOCK_TIMEOUT");

        std::this_thread::yield();
    }
}

void Lock::ReadUnlock()
{
    if ((_lockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0)
        CRASH("MULTIPLE_UNLOCK");
}
