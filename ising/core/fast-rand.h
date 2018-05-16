#ifndef ISING_CORE_FAST_RAND_H_
#define ISING_CORE_FAST_RAND_H_

#include "core/ising.h"

ISING_TOOLKIT_NAMESPACE_BEGIN

unsigned int FastRand();

void FastRandInitialize();
void FastRandInitialize(const int & seed);

ISING_TOOLKIT_NAMESPACE_END

#endif
