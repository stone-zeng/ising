#include "win-timing.h"

using namespace std;

Timing::Timing() { QueryPerformanceFrequency(&performance_freq_); }

void Timing::timingStart() { QueryPerformanceCounter(&counter_begin_time_); }
void Timing::timingEnd  () { QueryPerformanceCounter(&counter_end_time_);   }

double Timing::getRunTime() const
{
    // The default unit for the result is second.
    double counter_difference = static_cast<double>(counter_end_time_.QuadPart)
        - static_cast<double>(counter_begin_time_.QuadPart);
    return counter_difference / performance_freq_.QuadPart;
}

double Timing::getRunTime(const string & unit) const
{
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
