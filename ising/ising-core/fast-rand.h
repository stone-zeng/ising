#ifndef FATS_RAND_H
#define FATS_RAND_H

namespace Ising::Toolkit
{
    unsigned int fastRand();

    void fastRandInitialize();
    void fastRandInitialize(const int & seed);
}

#endif
