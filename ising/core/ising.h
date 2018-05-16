#ifndef ISING_CORE_ISING_H_
#define ISING_CORE_ISING_H_

#include <array>
#include <string>
#include <vector>

// Ising namespaces.
#define ISING_NAMESPACE_BEGIN         namespace ising {
#define ISING_NAMESPACE_END           }
#define ISING_TOOLKIT_NAMESPACE_BEGIN namespace ising { namespace toolkit {
#define ISING_TOOLKIT_NAMESPACE_END   } }

// Use OpenMP for parallel speedup.
#ifndef _DEBUG
#define ISING_PARALLEL
#endif
// Use pre-evaluated valued to replace `exp()`.
#define ISING_FAST_EXP

ISING_NAMESPACE_BEGIN

// A 2-dimensional matrix.
// Though Ising model is binary, I do not use std::vector<bool> due to some technical reasons.
// See https://stackoverflow.com/q/17794569/8479490.
typedef std::vector<std::vector<int>> Lattice2D;

// Store pre-evaluated Metropolis function values.
// 18 = (4 * 2 + 1) * 2 is the number of all the possible values of nearest sum.
typedef std::array<double, 18> ExpArray;

enum BoundaryCondition { kPeriodic, kFree };

struct LatticeSize
{
    LatticeSize() = default;
    LatticeSize(const std::size_t & size) : LatticeSize(size, size) {}
    LatticeSize(const std::size_t & x_size, const std::size_t & y_size) : x(x_size), y(y_size) {}

    std::size_t x;
    std::size_t y;
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

struct LatticeInfo
{
    LatticeInfo() {}
    LatticeInfo(const Lattice2D & lattice, const std::vector<Observable> & result) :
        lattice_data(lattice), observables(result) {}

    Lattice2D lattice_data;
    std::vector<Observable> observables;
};

ISING_NAMESPACE_END

#endif
