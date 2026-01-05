#pragma once

#include <cstdlib>
#include <exception>
#include <iostream>

/*---------------
      Lock
---------------*/
#define USE_MANY_LOCKS(count) Lock _locks[count];
#define USE_LOCK USE_MANY_LOCKS(1);
#define READ_LOCK_IDX(idx) ReadLockGuard readLockGuard_##idx(_locks[idx]);
#define READ_LOCK READ_LOCK_IDX(0);
#define WRITE_LOCK_IDX(idx) WriteLockGuard writeLockGuard_##idx(_locks[idx]);
#define WRITE_LOCK WRITE_LOCK_IDX(0);

/*---------------
      Crash
---------------*/
#define CRASH(cause)                                                           \
    {                                                                          \
        std::cerr << "\n========== CRASH ==========\n";                        \
        std::cerr << "Cause: " << cause << "\n";                               \
        std::cerr << "File: " << __FILE__ << "\n";                             \
        std::cerr << "Line: " << __LINE__ << "\n";                             \
        std::cerr << "===========================\n";                          \
        std::cerr.flush();                                                     \
        std::terminate();                                                      \
    }

#define ASSERT_CRASH(expr)                                                     \
    {                                                                          \
        if (!(expr)) {                                                         \
            std::cerr << "\n========== ASSERT FAILED ==========\n";            \
            std::cerr << "Expression: " << #expr << "\n";                      \
            std::cerr << "File: " << __FILE__ << "\n";                         \
            std::cerr << "Line: " << __LINE__ << "\n";                         \
            std::cerr << "===================================\n";              \
            std::cerr.flush();                                                 \
            std::terminate();                                                  \
        }                                                                      \
    }
