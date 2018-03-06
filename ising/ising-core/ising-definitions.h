#ifndef ISING_DEFINITIONS_H
#define ISING_DEFINITIONS_H

#include <vector>

namespace Ising
{
    typedef std::vector<std::vector<int>> Lattice2D;
    enum BoundaryTypes { periodic, free };

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
    };
}

#endif
