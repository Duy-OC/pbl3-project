#pragma once

#include <chrono>

using Clock = std::chrono::steady_clock;
using TimePoint = Clock::time_point;

inline double elapsedSeconds(TimePoint now, TimePoint before)
{
    return std::chrono::duration<double>(now - before).count();
}