#ifndef ISING_CORE_ISING_DEFINITIONS_H_
#define ISING_CORE_ISING_DEFINITIONS_H_

#include <array>
#include <vector>

#include "ising.h"

ISING_NAMESPACE_BEGIN

typedef std::vector<std::vector<int>> Lattice2D;
typedef std::array<double, 18>        ExpArray;

enum BoundaryConditions { kPeriodic, kFree };

struct LatticeSize
{
    size_t x;
    size_t y;
};

inline bool operator==(const LatticeSize & size_l, const LatticeSize & size_r)
{
    return size_l.x == size_r.x && size_l.y == size_r.y;
}

struct Quantity
{
    Quantity() : magnetic_dipole(0.0), energy(0.0) {}

    double magnetic_dipole;
    double energy;

    Quantity & operator/=(const double & scale)
    {
        magnetic_dipole /= scale;
        energy /= scale;
        return *this;
    }
    Quantity & operator+=(const Quantity & quantity)
    {
        magnetic_dipole += quantity.magnetic_dipole;
        energy += quantity.energy;
        return *this;
    }
    Quantity operator/(const double & scale)
    {
        Quantity result = *this;
        result /= scale;
        return result;
    }
};

ISING_NAMESPACE_END

#endif
