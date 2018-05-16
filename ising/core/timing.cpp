#include "core/timing.h"

using namespace std;

ISING_TOOLKIT_NAMESPACE_BEGIN

#ifdef _MSC_VER
Timing::Timing() { QueryPerformanceFrequency(&performance_freq_); }
#elif defined __GNUC__
Timing::Timing() : used_time_(0.0) { }
#endif

void Timing::TimingBegin()
{
#ifdef _MSC_VER
    QueryPerformanceCounter(&counter_begin_time_);
#elif defined __GNUC__
    gettimeofday(&begin_time_, NULL);
#endif
}

void Timing::TimingEnd()
{
#ifdef _MSC_VER
    QueryPerformanceCounter(&counter_end_time_);
#elif defined __GNUC__
    gettimeofday(&end_time_, NULL);
#endif
}

double Timing::GetRunningTime() const
{
    // The default unit for the result is second.
#ifdef _MSC_VER
    double counter_difference = static_cast<double>(counter_end_time_.QuadPart)
        - static_cast<double>(counter_begin_time_.QuadPart);
    return counter_difference / performance_freq_.QuadPart;
#elif defined __GNUC__
    double sec_difference       = static_cast<double>(end_time_.tv_sec)
        - static_cast<double>(begin_time_.tv_sec);
    double micro_sec_difference = static_cast<double>(end_time_.tv_usec)
        - static_cast<double>(begin_time_.tv_usec);
    return sec_difference + 1.0e-6 * micro_sec_difference;
#endif
}

double Timing::GetRunningTime(const string & unit) const
{
    if (unit == "ms")
        return 1.0e3 * GetRunningTime();
    if (unit == "us")
        return 1.0e6 * GetRunningTime();
    if (unit == "ns")
        return 1.0e9 * GetRunningTime();
    if (unit == "min")
        return GetRunningTime() / 60.0;
    return GetRunningTime();
}

ISING_TOOLKIT_NAMESPACE_END
