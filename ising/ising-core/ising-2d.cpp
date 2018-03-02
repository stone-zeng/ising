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

    Ising2D::Ising2D(const size_t & length)
    {
        x_length_ = length;
        y_length_ = length;
    }

    Ising2D::Ising2D(const size_t & x_length, const size_t & y_length)
    {
        x_length_ = x_length;
        y_length_ = y_length;
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

    void Ising2D_PBC::sweep(const double & beta, const double & magnetic_b)
    {
        for (auto i = 0; i != x_length_; ++i)
            for (auto j = 0; j != y_length_; ++j)
            {
                auto spin_sum = lattice_[i][yMinusOne(j)] + lattice_[i][yPlusOne(j)]
                    + lattice_[xMinusOne(i)][j] + lattice_[xPlusOne(i)][j];
                if (_isFlip(spin_sum, lattice_[i][j], magnetic_b, beta))
                    lattice_[i][j] *= -1;
            }
    }

    void Ising2D_FBC::sweep(const double & beta, const double & magnetic_b)
    {
        for (auto i = 1; i != x_length_ + 1; ++i)
            for (auto j = 1; j != y_length_ + 1; ++j)
            {
                auto spin_sum = lattice_[i][j - 1] + lattice_[i][j + 1]
                    + lattice_[i - 1][j] + lattice_[i + 1][j];
                if (_isFlip(spin_sum, lattice_[i][j], magnetic_b, beta))
                    lattice_[i][j] *= -1;
            }
    }
}
