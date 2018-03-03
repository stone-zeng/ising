#include "ising-2d.h"

using namespace std;
using namespace Ising::Toolkit;

namespace Ising
{
    inline double _metropolisFunction(const double & energy, const double & beta)
    {
        auto boltzmann_probability = exp(-beta * energy);
        return boltzmann_probability < 1.0 ? boltzmann_probability : 1.0;
    }

    inline bool _isFlip(const int & spin_sum, const int & spin_value,
        const double & magnetic_b, const double & beta)
    {
        auto energy_difference = 2 * (spin_sum + magnetic_b) * spin_value;
        auto flip_probability = _metropolisFunction(energy_difference, beta);
        return (static_cast<double>(fastRand()) / RAND_MAX) < flip_probability;
    }

    Ising2D::Ising2D(const size_t & length) : Ising2D(length, length) {}

    Ising2D::Ising2D(const size_t & x_length, const size_t & y_length) :
        x_length_(x_length), y_length_(y_length) {}

    void Ising2D::sweep(const double & beta, const double & magnetic_b)
    {
        for (auto i = x_begin_index_; i != x_end_index_; ++i)
            for (auto j = y_begin_index_; j != y_end_index_; ++j)
            {
                auto spin_sum = nearestSum(i, j);
                if (_isFlip(spin_sum, lattice_[i][j], magnetic_b, beta))
                    lattice_[i][j] *= -1;
            }
    }

    Quantity Ising2D::analysis(const double & magnetic_b) const
    {
        Quantity quantity;

        for (auto i = x_begin_index_; i != x_end_index_; ++i)
            for (auto j = y_begin_index_; j != y_end_index_; ++j)
            {
                quantity.magnetic_dipole += lattice_[i][j];
                auto spin_sum = nearestSum(i, j);
                quantity.energy -= (spin_sum + magnetic_b) * lattice_[i][j];
            }
        quantity /= static_cast<double>(x_length_ * y_length_);
        return quantity;
    }

    Quantity Ising2D::evaluate(const double & beta, const double & magnetic_b, const size_t & steps,
        const size_t & n_ensemble, const size_t & n_delta)
    {
        // Sweep.
        for (auto i = 0; i != steps - n_ensemble; ++i)
            sweep(beta, magnetic_b);

        // Sweep and analysis.
        // n_delta is used to avoid correlation between successive configurations.
        auto count = 0;
        Quantity quantity;
        for (auto i = steps - n_ensemble - 1; i != steps; ++i)
        {
            sweep(beta, magnetic_b);
            if (count == n_delta)
            {
                quantity += analysis(magnetic_b);
                count = 0;
            }
            count += 1;
        }

        // Normalize.
        quantity /= static_cast<double>(n_ensemble / n_delta);
        return quantity;
    }

    vector<int> Ising2D::renormalize(const size_t & x_scale, const size_t & y_scale)
    {
        vector<int> v;
        return v;

        /*
        size_t x_length_renormalized = x_length_ / x_scale;
        size_t y_length_renormalized = y_length_ / y_scale;

        vector<int> result;
        result.resize(x_length_renormalized * y_length_renormalized);

        vector<vector<int>> local_lattice;
        local_lattice.resize(x_scale);
        for (auto i = local_lattice.begin(); i != local_lattice.end(); ++i)
            i->resize(y_scale);

        for (auto i = 0; i != x_length_renormalized; ++i)
            for (auto j = 0; j != y_length_renormalized; ++j)
            {
                vector<int> local_lattice;
                local_lattice.resize(x_scale * y_scale);
                for (auto k = 0; k != x_scale; ++k)
                    for (auto l = 0; l != y_scale; ++l)
                        // Use "+1" to skip the boundary.
                        local_lattice[y_scale * k + l]
                        = lattice_[x_scale * i + k + 1][y_scale * j + l + 1];

                for (auto t = 0; t != x_scale * y_scale; ++t)
                    result[y_length_renormalized * i + j]
                    += (local_lattice[t] + 1) / 2 * static_cast<int>(pow(2, t));
            }

        return result;
        */
    }

    void Ising2D::show()
    {
        for (auto i = lattice_.begin(); i != lattice_.end(); ++i)
        {
            for (auto j = i->begin(); j != i->end(); ++j)
                cout << *j << " ";
            cout << endl;
        }
    }

    Ising2D_PBC::Ising2D_PBC(const size_t & length) : Ising2D_PBC(length, length) {}
    Ising2D_PBC::Ising2D_PBC(const size_t & x_length, const size_t & y_length) :
        Ising2D(x_length, y_length)
    {
        x_begin_index_ = 0;
        y_begin_index_ = 0;
        x_end_index_ = x_length;
        y_end_index_ = y_length;
    }

    Ising2D_FBC::Ising2D_FBC(const size_t & length) : Ising2D_FBC(length, length) {}
    Ising2D_FBC::Ising2D_FBC(const size_t & x_length, const size_t & y_length) :
        Ising2D(x_length, y_length)
    {
        x_begin_index_ = 1;
        y_begin_index_ = 1;
        x_end_index_ = x_length + 1;
        y_end_index_ = y_length + 1;
    }

    void Ising2D_PBC::initialize()
    {
        lattice_.resize(x_length_);
        for (int i = 0; i != x_length_; ++i)
        {
            lattice_[i].resize(y_length_);
            for (int j = 0; j != y_length_; ++j)
                lattice_[i][j] = 1;
        }
    }

    void Ising2D_FBC::initialize()
    {
        // Add zero padding to the original lattice.
        lattice_.resize(x_length_ + 2);
        for (auto i = lattice_.begin(); i != lattice_.end(); ++i)
            i->resize(y_length_ + 2);
        for (auto i = 1; i != x_length_ + 1; ++i)
            for (auto j = 1; j != y_length_ + 1; ++j)
                lattice_[i][j] = 1;
    }
}
