#ifndef ISING_CORE_FAST_RAND_H_
#define ISING_CORE_FAST_RAND_H_

#include "ising.h"

ISING_TOOLKIT_NAMESPACE_BEGIN

unsigned int fastRand();

void fastRandInitialize();
void fastRandInitialize(const int & seed);

ISING_TOOLKIT_NAMESPACE_END

#endif
