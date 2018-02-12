#ifndef FATS_RAND_H
#define FATS_RAND_H

#include <iostream>
#include <random>
#include <vector>
#include <cstdlib>
#include <ctime>
#include "win-timing.h"

inline long long int _getTimeInt()
{
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return t.QuadPart;
}

static unsigned int g_seed = static_cast<unsigned int>(_getTimeInt());

inline unsigned int fastRand()
{
    g_seed = (214013 * g_seed + 2531011);
    return (g_seed >> 16) & 0x7fff;
}

inline void _randomTest(const size_t & size)
{
    std::vector<double> v1(size);
    std::vector<double> v2(size);
    std::vector<double> v3(size);

    Timing clock;

    // C standard random integer.
    clock.timingStart();
    for (auto & i : v1)
        i = static_cast<double>(std::rand()) / RAND_MAX;
    clock.timingEnd();
    std::cerr << "C rand():  " << clock.getRunTime("ms") << "ms." << std::endl;

    // C++ 11 random integer.
    static std::default_random_engine engine;
    static std::uniform_real_distribution<> dist{ 0, 1 };
    engine.seed(static_cast<unsigned int>(std::time(NULL)));
    clock.timingStart();
    for (auto & i : v2)
        i = dist(engine);
    clock.timingEnd();
    std::cerr << "C++ 11:    " << clock.getRunTime("ms") << "ms." << std::endl;

    // My fast random integer.
    clock.timingStart();
    for (auto & i : v3)
        i = static_cast<double>(fastRand()) / RAND_MAX;
    clock.timingEnd();
    std::cerr << "fastRand():" << clock.getRunTime("ms") << "ms." << std::endl;

    // Output.
    //for (auto i = 0; i != size; ++i)
    //    std::cout << v1[i] << "," << v2[i] << "," << v3[i] << std::endl;
}

#endif
