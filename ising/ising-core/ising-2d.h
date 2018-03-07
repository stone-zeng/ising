#ifndef ISING_CORE_ISING_2D_H_
#define ISING_CORE_ISING_2D_H_

#include <iostream>
#include <vector>

#include "ising.h"
#include "ising-definitions.h"

ISING_NAMESPACE_BEGIN

// Abstract base class for general 2D Ising model.
class Ising2D
{
public:
    Ising2D() = default;
    Ising2D(const size_t & length);
    Ising2D(const size_t & x_length, const size_t & y_length);

    //std::vector<int> Renormalize(const size_t & x_scale, const size_t & y_scale);

    // Initialize all the spins to be +1.
    virtual void Initialize() = 0;

    // Sweep through the lattice once using Metropolis algorithm.
    void Sweep(const double & beta, const double & magnetic_b);

    // Calculate physical quantities.
    Quantity Analysis(const double & magnetic_b) const;

    Quantity Evaluate(const double & beta, const double & magnetic_b, const size_t & steps,
        const size_t & n_ensemble, const size_t & n_delta = 1);

    std::vector<int> Renormalize(const size_t & x_scale, const size_t & y_scale);

    // Show lattice (including zero padding if existing).
    void Show();

protected:
    const size_t x_length_;
    const size_t y_length_;

    size_t x_begin_index_;
    size_t x_end_index_;
    size_t y_begin_index_;
    size_t y_end_index_;

    Lattice2D lattice_;

    inline virtual int NearestSum(const size_t & x, const size_t & y) const = 0;
};

// 2D Ising model with periodic boundary condition.
class Ising2D_PBC : public Ising2D
{
public:
    using Ising2D::Ising2D;
    Ising2D_PBC(const size_t & length);
    Ising2D_PBC(const size_t & x_length, const size_t & y_length);

    void Initialize() override;

private:
    inline size_t XPlusOne (const size_t & x) const { return (x == x_length_ - 1 ? 0 : x + 1); }
    inline size_t XMinusOne(const size_t & x) const { return (x == 0 ? x_length_ - 1 : x - 1); }
    inline size_t YPlusOne (const size_t & y) const { return (y == y_length_ - 1 ? 0 : y + 1); }
    inline size_t YMinusOne(const size_t & y) const { return (y == 0 ? y_length_ - 1 : y - 1); }

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
    Ising2D_FBC(const size_t & length);
    Ising2D_FBC(const size_t & x_length, const size_t & y_length);

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
