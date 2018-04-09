#include "ising-core/win-timing.h"

using namespace std;

ISING_TOOLKIT_NAMESPACE_BEGIN

Timing::Timing() { QueryPerformanceFrequency(&performance_freq_); }

void Timing::TimingStart() { QueryPerformanceCounter(&counter_begin_time_); }
void Timing::TimingEnd  () { QueryPerformanceCounter(&counter_end_time_);   }

double Timing::GetRunningTime() const
{
    // The default unit for the result is second.
    double counter_difference = static_cast<double>(counter_end_time_.QuadPart)
        - static_cast<double>(counter_begin_time_.QuadPart);
    return counter_difference / performance_freq_.QuadPart;
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
