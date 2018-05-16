#include "core/fast-rand.h"

#ifdef _MSC_VER
#include <Windows.h>
#elif defined __GNUC__
#include <sys/time.h>
#endif

#include "core/ising.h"
#include "core/timing.h"

using namespace std;

ISING_TOOLKIT_NAMESPACE_BEGIN

static unsigned int _fast_rand_seed = 0;

unsigned int FastRand()
{
    // See https://stackoverflow.com/a/3747462/8479490.
    _fast_rand_seed = (214013 * _fast_rand_seed + 2531011);
    return (_fast_rand_seed >> 16) & 0x7fff;
}

time_t _GetTime()
{
#ifdef _MSC_VER
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return t.QuadPart;
#elif defined __GNUC__
    timeval t;
    gettimeofday(&t, NULL);
    return t.tv_usec;
#endif
}

void FastRandInitialize() { _fast_rand_seed = static_cast<unsigned int>(_GetTime()); }
void FastRandInitialize(const int & seed) { _fast_rand_seed = seed; }

ISING_TOOLKIT_NAMESPACE_END
