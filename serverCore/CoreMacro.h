#pragma once

#include <cstdlib>
#include <exception>
#include <iostream>

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
        if (!(expr))                                                           \
        {                                                                      \
            std::cerr << "\n========== ASSERT FAILED ==========\n";            \
            std::cerr << "Expression: " << #expr << "\n";                      \
            std::cerr << "File: " << __FILE__ << "\n";                         \
            std::cerr << "Line: " << __LINE__ << "\n";                         \
            std::cerr << "===================================\n";              \
            std::cerr.flush();                                                 \
            std::terminate();                                                  \
        }                                                                      \
    }
