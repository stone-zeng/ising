#include "ising-2d.h"

#include <array>
#include <iostream>
#include <string>
#include <vector>

#include "fast-rand.h"
#include "ising.h"
#include "ising-definitions.h"

using namespace std;
using namespace ising::toolkit;

ISING_NAMESPACE_BEGIN

double _MetropolisFunction(const double & energy, const double & beta)
{
    auto boltzmann_probability = exp(-beta * energy);
    return boltzmann_probability < 1.0 ? boltzmann_probability : 1.0;
}

bool _IsFlip(const int & spin_sum, const int & spin_value,
    const double & magnetic_h, const double & beta)
{
    auto energy_difference = 2 * (spin_sum + magnetic_h) * spin_value;
    auto flip_probability = _MetropolisFunction(energy_difference, beta);
    return (static_cast<double>(FastRand()) / RAND_MAX) < flip_probability;
}

Ising2D::Ising2D(const size_t & size) : Ising2D(size, size) {}

Ising2D::Ising2D(const size_t & x_size, const size_t & y_size) :
    x_size_(x_size), y_size_(y_size) {}

Ising2D::Ising2D(const LatticeSize & size) : Ising2D(size.x, size.y) {}

void Ising2D::Sweep(const double & beta, const double & magnetic_h)
{
    for (auto i = x_begin_index_; i != x_end_index_; ++i)
        for (auto j = y_begin_index_; j != y_end_index_; ++j)
        {
            auto spin_sum = NearestSum(i, j);
            if (_IsFlip(spin_sum, lattice_[i][j], magnetic_h, beta))
                lattice_[i][j] *= -1;
        }
}

void Ising2D::Sweep(const ExpArray & exp_array)
{
    for (auto i = x_begin_index_; i != x_end_index_; ++i)
        for (auto j = y_begin_index_; j != y_end_index_; ++j)
        {
            auto spin_sum = NearestSum(i, j);
            auto flip_probability = exp_array[(spin_sum - 5 * lattice_[i][j] + 9) / 2];
            if (static_cast<double>(FastRand()) / RAND_MAX < flip_probability)
                lattice_[i][j] *= -1;
        }
}

Quantity Ising2D::Analysis(const double & magnetic_h) const
{
    Quantity quantity;
    for (auto i = x_begin_index_; i != x_end_index_; ++i)
        for (auto j = y_begin_index_; j != y_end_index_; ++j)
        {
            quantity.magnetic_dipole += lattice_[i][j];
            auto spin_sum = NearestSum(i, j);
            quantity.energy -= (spin_sum + magnetic_h) * lattice_[i][j];
        }
    return quantity / static_cast<double>(x_size_ * y_size_);
}

Quantity Ising2D::Evaluate(const double & beta, const double & magnetic_h,
    const size_t & iterations, const size_t & n_ensemble, const size_t & n_delta)
{
#ifdef ISING_FAST_EXP
    ExpArray kExpArray =
    {
        exp(-beta * (-4.0 + magnetic_h)),
        exp(-beta * (-2.0 + magnetic_h)),
        exp(-beta * ( 0.0 + magnetic_h)),
        exp(-beta * ( 2.0 + magnetic_h)),
        exp(-beta * ( 4.0 + magnetic_h)),
        exp( beta * (-4.0 + magnetic_h)),
        exp( beta * (-2.0 + magnetic_h)),
        exp( beta * ( 0.0 + magnetic_h)),
        exp( beta * ( 2.0 + magnetic_h)),
        exp( beta * ( 4.0 + magnetic_h))
    };
    for (auto & i : kExpArray)
        i = i < 1.0 ? i : 1.0;
#endif

    // Sweep.
    for (auto i = 0; i != iterations - n_ensemble; ++i)
#ifdef ISING_FAST_EXP
        Sweep(kExpArray);
#else
        Sweep(beta, magnetic_h);
#endif

    // Sweep and analysis.
    // `n_delta` is used to avoid correlation between successive configurations.
    auto count = 0;
    Quantity quantity;
    for (auto i = iterations - n_ensemble - 1; i != iterations; ++i)
    {
#ifdef ISING_FAST_EXP
        Sweep(kExpArray);
#else
        Sweep(beta, magnetic_h);
#endif
        if (count == n_delta)
        {
            quantity += Analysis(magnetic_h);
            count = 0;
        }
        count += 1;
    }
    // Normalize.
    return quantity / static_cast<double>(n_ensemble / n_delta);
}

/*
vector<int> Ising2D::Renormalize(const size_t & x_scale, const size_t & y_scale)
{
    size_t x_size_renormalized = x_size_ / x_scale;
    size_t y_size_renormalized = y_size_ / y_scale;

    vector<int> result;
    result.resize(x_size_renormalized * y_size_renormalized);

    vector<vector<int>> local_lattice;
    local_lattice.resize(x_scale);
    for (auto i = local_lattice.begin(); i != local_lattice.end(); ++i)
        i->resize(y_scale);

    for (auto i = 0; i != x_size_renormalized; ++i)
        for (auto j = 0; j != y_size_renormalized; ++j)
        {
            vector<int> local_lattice;
            local_lattice.resize(x_scale * y_scale);
            for (auto k = 0; k != x_scale; ++k)
                for (auto l = 0; l != y_scale; ++l)
                    // Use "+1" to skip the boundary.
                    local_lattice[y_scale * k + l]
                    = lattice_[x_scale * i + k + 1][y_scale * j + l + 1];

            for (auto t = 0; t != x_scale * y_scale; ++t)
                result[y_size_renormalized * i + j]
                += (local_lattice[t] + 1) / 2 * static_cast<int>(pow(2, t));
        }

    return result;
}
*/

void Ising2D::Show() const
{
    for (auto i = lattice_.begin(); i != lattice_.end(); ++i)
    {
        for (auto j = i->begin(); j != i->end(); ++j)
            cout << *j << " ";
        cout << endl;
    }
}

string Ising2D::ShowRow(const size_t & row) const
{
    string result;
    for (auto i : lattice_[row])
        result += to_string(i) + " ";
    return result;
}

Ising2D_PBC::Ising2D_PBC(const LatticeSize & size) : Ising2D_PBC(size.x, size.y) {}
Ising2D_PBC::Ising2D_PBC(const size_t & size) : Ising2D_PBC(size, size) {}
Ising2D_PBC::Ising2D_PBC(const size_t & x_size, const size_t & y_size) :
    Ising2D(x_size, y_size)
{
    x_begin_index_ = 0;
    y_begin_index_ = 0;
    x_end_index_ = x_size;
    y_end_index_ = y_size;
}

Ising2D_FBC::Ising2D_FBC(const LatticeSize & size) : Ising2D_FBC(size.x, size.y) {}
Ising2D_FBC::Ising2D_FBC(const size_t & size) : Ising2D_FBC(size, size) {}
Ising2D_FBC::Ising2D_FBC(const size_t & x_size, const size_t & y_size) :
    Ising2D(x_size, y_size)
{
    x_begin_index_ = 1;
    y_begin_index_ = 1;
    x_end_index_ = x_size + 1;
    y_end_index_ = y_size + 1;
}

void Ising2D_PBC::Initialize()
{
    lattice_.resize(x_size_);
    for (int i = 0; i != x_size_; ++i)
    {
        lattice_[i].resize(y_size_);
        for (int j = 0; j != y_size_; ++j)
            lattice_[i][j] = 1;
    }
}

void Ising2D_FBC::Initialize()
{
    // Add zero padding to the original lattice.
    lattice_.resize(x_size_ + 2);
    for (auto i = lattice_.begin(); i != lattice_.end(); ++i)
        i->resize(y_size_ + 2);
    for (auto i = 1; i != x_size_ + 1; ++i)
        for (auto j = 1; j != y_size_ + 1; ++j)
            lattice_[i][j] = 1;
}

ISING_NAMESPACE_END
