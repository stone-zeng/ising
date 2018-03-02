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
    Timing();

    void timingStart();
    void timingEnd();

    // "Run time" equals the difference of two counters over counter frequency.
    // run_time = end_time - start_time
    //          = (`counter_begin_time` - `counter_begin_time`) / `performance_freq`
    double getRunTime() const;
    double getRunTime(const std::string & unit) const;

private:
    LARGE_INTEGER performance_freq_;
    LARGE_INTEGER counter_begin_time_, counter_end_time_;
};

#endif
