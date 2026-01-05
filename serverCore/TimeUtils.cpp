#include "TimeUtils.h"
#include <chrono>

uint64_t TimeUtils::GetTickCount64() {
    using namespace std::chrono;

    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch())
        .count();
}
