#ifndef WIN_TIMING_H
#define WIN_TIMING_H

#include <string>
#include <Windows.h>

// High precision timing using Windows API.
// Usage:
//     Timing my_timer;
//     my_timer.timingStart();
//     /* Some codes here. */
//     my_timer.timingEnd();
//
// Then use `my_timer.getRunTime()` or `my_timer.getRunTime("<Units>")`
//   to get the running time (in seconds).
// "Units" can be "ms" (=millisecond), "us" (=microsecond), "ns" (=nanosecond)
//   or "min" (=minute).
class Timing
{
public:
    inline Timing() { QueryPerformanceFrequency(&performance_freq_); }
    inline void timingStart() { QueryPerformanceCounter(&counter_begin_time_); }
    inline void timingEnd() { QueryPerformanceCounter(&counter_end_time_); }

    // "Run time" equals the difference of two counters over counter frequency.
    // run_time = end_time - start_time
    //          = (`counter_begin_time` - `counter_begin_time`) / `performance_freq`
    inline double getRunTime();
    inline double getRunTime(const std::string &unit);

private:
    LARGE_INTEGER performance_freq_;
    LARGE_INTEGER counter_begin_time_, counter_end_time_;
};

inline double Timing::getRunTime()
{
    // The default unit for the result is second.
    double counter_difference = static_cast<double>(counter_end_time_.QuadPart)
        - static_cast<double>(counter_begin_time_.QuadPart);
    return counter_difference / performance_freq_.QuadPart;
}

inline double Timing::getRunTime(const std::string &unit)
{
    // The default unit for the result is second.
    if (unit == "ms")
        return 1000.0 * getRunTime();
    if (unit == "us")
        return 1000000.0 * getRunTime();
    if (unit == "ns")
        return 1000000000.0 * getRunTime();
    if (unit == "min")
        return getRunTime() / 60.0;
    return getRunTime();
}

#endif
