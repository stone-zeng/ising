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

struct Observable
{
    Observable() :
        magnetic_dipole(0.0),
        energy(0.0),
        magnetic_dipole_abs(0.0),
        magnetic_dipole_square(0.0),
        energy_square(0.0) {}

    double magnetic_dipole;
    double energy;
    double magnetic_dipole_abs;
    double magnetic_dipole_square;
    double energy_square;

    Observable & operator/=(const double & scale)
    {
        magnetic_dipole        /= scale;
        energy                 /= scale;
        magnetic_dipole_abs    /= scale;
        magnetic_dipole_square /= scale;
        energy_square          /= scale;
        return *this;
    }

    Observable & operator+=(const Observable & observable)
    {
        magnetic_dipole        += observable.magnetic_dipole;
        energy                 += observable.energy;
        magnetic_dipole_abs    += observable.magnetic_dipole_abs;
        magnetic_dipole_square += observable.magnetic_dipole_square;
        energy_square          += observable.energy_square;
        return *this;
    }

    Observable operator/(const double & scale)
    {
        Observable result = *this;
        result /= scale;
        return result;
    }
};

ISING_NAMESPACE_END

#endif
