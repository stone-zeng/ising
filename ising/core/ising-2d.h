#ifndef ISING_CORE_ISING_2D_H_
#define ISING_CORE_ISING_2D_H_

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "core/ising.h"

ISING_NAMESPACE_BEGIN

// Abstract base class for general 2D Ising model.
// May not be used directly.
class Ising2D
{
public:
    Ising2D() = default;
    Ising2D(const LatticeSize & size);
    Ising2D(const size_t & size);
    Ising2D(const size_t & x_size, const size_t & y_size);

    // Initialize all the spins to be +1.
    virtual void Initialize() = 0;

    // Sweep through the lattice once using Metropolis algorithm.
    void Sweep(const double & beta, const double & magnetic_h);
    void Sweep(const ExpArray & exp_array);

    // Calculate physical quantities.
    Observable Analysis(const double & magnetic_h) const;

    // A complete evaluation process. Should be initialized before!
    Observable Evaluate(const double & beta, const double & magnetic_h,
        const size_t & iterations, const size_t & n_ensemble, const size_t & n_delta = 1);

    // For lattice data generating and convergence analysis.
    LatticeInfo EvaluateLatticeData(const double & beta, const double & magnetic_h,
        const size_t & iterations);

    // Reshape the lattice to be a 1D vector.
    // std::vector<int> Renormalize(const size_t & x_scale, const size_t & y_scale);

    // Show lattice (including zero padding if existing).
    void Show() const;
    std::string ShowRow(const size_t & row) const;

protected:
    const size_t x_size_;
    const size_t y_size_;

    // Used for loops concerning boundary conditions.
    // They cannot be initialized directly. Use non-const type to be set value afterwards.
    size_t x_begin_index_;
    size_t x_end_index_;
    size_t y_begin_index_;
    size_t y_end_index_;

    Lattice2D lattice_;

    // Sum over the nearest spins.
    // It's pure virtual because of the different boundary conditions.
    virtual int NearestSum(const size_t & x, const size_t & y) const = 0;

private:
    // Pre-evaluate the Metropolis function values (`exp()`).
    inline ExpArray InitializeExpArray(const double & beta, const double & magnetic_h)
    {
        ExpArray exp_array =
        {
            // spin = +1
            std::exp(-2 * beta * (-4.0 + magnetic_h)),
            std::exp(-2 * beta * (-3.0 + magnetic_h)),
            std::exp(-2 * beta * (-2.0 + magnetic_h)),
            std::exp(-2 * beta * (-1.0 + magnetic_h)),
            std::exp(-2 * beta * ( 0.0 + magnetic_h)),
            std::exp(-2 * beta * ( 1.0 + magnetic_h)),
            std::exp(-2 * beta * ( 2.0 + magnetic_h)),
            std::exp(-2 * beta * ( 3.0 + magnetic_h)),
            std::exp(-2 * beta * ( 4.0 + magnetic_h)),
            // spin = -1
            std::exp( 2 * beta * (-4.0 + magnetic_h)),
            std::exp( 2 * beta * (-3.0 + magnetic_h)),
            std::exp( 2 * beta * (-2.0 + magnetic_h)),
            std::exp( 2 * beta * (-1.0 + magnetic_h)),
            std::exp( 2 * beta * ( 0.0 + magnetic_h)),
            std::exp( 2 * beta * ( 1.0 + magnetic_h)),
            std::exp( 2 * beta * ( 2.0 + magnetic_h)),
            std::exp( 2 * beta * ( 3.0 + magnetic_h)),
            std::exp( 2 * beta * ( 4.0 + magnetic_h))
        };
        for (auto & i : exp_array)
            i = i < 1.0 ? i : 1.0;
        return exp_array;
    }
};

// 2D Ising model with periodic boundary condition.
class Ising2D_PBC : public Ising2D
{
public:
    using Ising2D::Ising2D;
    Ising2D_PBC(const LatticeSize & size);
    Ising2D_PBC(const size_t & size);
    Ising2D_PBC(const size_t & x_size, const size_t & y_size);

    void Initialize() override;

private:
    inline size_t XPlusOne (const size_t & x) const { return (x == x_size_ - 1 ? 0 : x + 1); }
    inline size_t XMinusOne(const size_t & x) const { return (x == 0 ? x_size_ - 1 : x - 1); }
    inline size_t YPlusOne (const size_t & y) const { return (y == y_size_ - 1 ? 0 : y + 1); }
    inline size_t YMinusOne(const size_t & y) const { return (y == 0 ? y_size_ - 1 : y - 1); }

    inline int NearestSum(const size_t & x, const size_t & y) const override
    {
        return lattice_[x][YMinusOne(y)] + lattice_[x][YPlusOne(y)]
             + lattice_[XMinusOne(x)][y] + lattice_[XPlusOne(x)][y];
    }
};

// 2D Ising model with free boundary condition (with zero padding).
class Ising2D_FBC : public Ising2D
{
public:
    using Ising2D::Ising2D;
    Ising2D_FBC(const LatticeSize & size);
    Ising2D_FBC(const size_t & size);
    Ising2D_FBC(const size_t & x_size, const size_t & y_size);

    void Initialize() override;

private:
    inline int NearestSum(const size_t & x, const size_t & y) const override
    {
        return lattice_[x][y - 1] + lattice_[x][y + 1]
             + lattice_[x - 1][y] + lattice_[x + 1][y];
    }
};

ISING_NAMESPACE_END

#endif
