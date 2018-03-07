#include "fast-rand.h"

#include <Windows.h>

#include "ising.h"
#include "win-timing.h"

using namespace std;

ISING_TOOLKIT_NAMESPACE_BEGIN

static unsigned int _g_seed = 0;

unsigned int fastRand()
{
    _g_seed = (214013 * _g_seed + 2531011);
    return (_g_seed >> 16) & 0x7fff;
}

long long int _getTimeInt()
{
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return t.QuadPart;
}

void fastRandInitialize() { _g_seed = static_cast<unsigned int>(_getTimeInt()); }
void fastRandInitialize(const int & seed) { _g_seed = seed; }

ISING_TOOLKIT_NAMESPACE_END
