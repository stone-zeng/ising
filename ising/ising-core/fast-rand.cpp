#include "ising-core/fast-rand.h"

#include <Windows.h>

#include "ising-core/ising.h"
#include "ising-core/win-timing.h"

using namespace std;

ISING_TOOLKIT_NAMESPACE_BEGIN

static unsigned int _fast_rand_seed = 0;

unsigned int FastRand()
{
    // See https://stackoverflow.com/a/3747462/8479490.
    _fast_rand_seed = (214013 * _fast_rand_seed + 2531011);
    return (_fast_rand_seed >> 16) & 0x7fff;
}

long long int _GetTime()
{
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return t.QuadPart;
}

void FastRandInitialize() { _fast_rand_seed = static_cast<unsigned int>(_GetTime()); }
void FastRandInitialize(const int & seed) { _fast_rand_seed = seed; }

ISING_TOOLKIT_NAMESPACE_END
