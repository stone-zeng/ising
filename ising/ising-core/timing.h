#ifndef ISING_CORE_TIMING_H_
#define ISING_CORE_TIMING_H_

#include <string>
#ifdef _MSC_VER
#include <Windows.h>
#elif defined __GNUC__
#include <sys/time.h>
#endif

#include "ising-core/ising.h"

ISING_TOOLKIT_NAMESPACE_BEGIN

// High precision timing.
// Usage:
//     Timing my_timer;
//     my_timer.TimingBegin();
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

    void TimingBegin();
    void TimingEnd();

    double GetRunningTime() const;
    double GetRunningTime(const std::string & unit) const;

private:
#ifdef _MSC_VER
    LARGE_INTEGER performance_freq_;
    LARGE_INTEGER counter_begin_time_;
    LARGE_INTEGER counter_end_time_;
#elif defined __GNUC__
    double  used_time_;
    timeval begin_time_;
    timeval end_time_;
#endif
};

ISING_TOOLKIT_NAMESPACE_END

#endif
