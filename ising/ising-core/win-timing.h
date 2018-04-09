#ifndef ISING_CORE_WIN_TIMING_H_
#define ISING_CORE_WIN_TIMING_H_

#include <string>
#include <Windows.h>

#include "ising-core/ising.h"

ISING_TOOLKIT_NAMESPACE_BEGIN

// High precision timing using Windows API.
// Usage:
//     Timing my_timer;
//     my_timer.TimingStart();
//     /* Some codes here. */
//     my_timer.TimingEnd();
//
// Then use `my_timer.GetRunningTime()` or `my_timer.GetRunningTime("<Units>")`
//   to get the running time (in seconds).
// "Units" can be "ms" (=millisecond), "us" (=microsecond), "ns" (=nanosecond)
//   or "min" (=minute).
class Timing
{
public:
    Timing();

    void TimingStart();
    void TimingEnd();

    // "Run time" equals the difference of two counters over counter frequency.
    // run_time = end_time - start_time
    //          = (`counter_begin_time` - `counter_begin_time`) / `performance_freq`
    double GetRunningTime() const;
    double GetRunningTime(const std::string & unit) const;

private:
    LARGE_INTEGER performance_freq_;
    LARGE_INTEGER counter_begin_time_;
    LARGE_INTEGER counter_end_time_;
};

ISING_TOOLKIT_NAMESPACE_END

#endif
