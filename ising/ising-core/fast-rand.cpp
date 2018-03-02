#include <iostream>
#include <random>
#include <vector>
#include <cstdlib>
#include <ctime>

#include "fast-rand.h"
#include "win-timing.h"

using namespace std;

static unsigned int g_seed = 0;

unsigned int fastRand()
{
    g_seed = (214013 * g_seed + 2531011);
    return (g_seed >> 16) & 0x7fff;
}

long long int _getTimeInt()
{
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return t.QuadPart;
}

void fastRandInitialize() { g_seed = static_cast<unsigned int>(_getTimeInt()); }
void fastRandInitialize(const int & seed) { g_seed = seed; }
