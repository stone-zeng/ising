#ifndef ISING_2D_H
#define ISING_2D_H

#include <iostream>
#include <vector>
#include <cstdlib>
#include "fast-rand.h"

struct Quantity
{
    Quantity()
    {
        magnetic_dipole = 0.0;
        energy = 0.0;
    }

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

class Ising2D
{
public:
    Ising2D();
    Ising2D(const size_t & x_length, const size_t & y_length);
    ~Ising2D();

    Quantity evaluate(const double & beta, const double & magnetic_b, const size_t & steps,
        const size_t & n_ensemble, const size_t & n_delta = 1);
    Quantity analysis(const double & magnetic_b);
    void show();

private:
    size_t x_length_;
    size_t y_length_;

    std::vector<std::vector<int>> lattice_;

    void initialize();
    void sweep(const double & beta, const double & magnetic_b);

    inline double metropolisFunction(const double & energy, const double & beta)
    {
        auto boltzmann_probability = std::exp(-beta * energy);
        return boltzmann_probability < 1.0 ? boltzmann_probability : 1.0;
    }

    std::default_random_engine random_engine_;
    std::uniform_real_distribution<> random_dist_0_1_{ 0, 1 };
    inline double randReal_0_1()
    {
        return random_dist_0_1_(random_engine_);
    }
};

Ising2D::Ising2D()
{}

Ising2D::Ising2D(const size_t & x_length, const size_t & y_length)
{
    x_length_ = x_length;
    y_length_ = y_length;

    // Add zero padding to the original lattice.
    lattice_.resize(x_length_ + 2);
    for (auto i = lattice_.begin(); i != lattice_.end(); ++i)
        i->resize(y_length_ + 2);

    initialize();
}

Ising2D::~Ising2D()
{}

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

Quantity Ising2D::analysis(const double & magnetic_b)
{
    Quantity quantity;

    for (auto i = 1; i != x_length_ + 1; ++i)
        for (auto j = 1; j != y_length_ + 1; ++j)
        {
            quantity.magnetic_dipole += lattice_[i][j];
            auto s_sum = lattice_[i][j - 1] + lattice_[i][j + 1]
                       + lattice_[i - 1][j] + lattice_[i + 1][j];
            quantity.energy -= (s_sum + magnetic_b) * lattice_[i][j];
        }
    //for (auto i : lattice_)
    //    for (auto j : i)
    //        m += j;
    quantity /= static_cast<double>(x_length_ * y_length_);
    return quantity;
}

void Ising2D::show()
{
    //for (auto i = lattice_.begin() + 1; i != lattice_.end() - 1; ++i)
    //{
    //    for (auto j = i->begin() + 1; j != i->end() - 1; ++j)
    //        std::cout << *j << " ";
    //    std::cout << std::endl;
    //}
    for (auto i = 1; i != x_length_ + 1; ++i)
    {
        for (auto j = 1; j != y_length_ + 1; ++j)
            std::cout << lattice_[i][j] << " ";
        std::cout << std::endl;
    }
}

void Ising2D::initialize()
{
    //for (auto i = lattice_.begin() + 1; i != lattice_.end() - 1; ++i)
    //    for (auto j = i->begin() + 1; j != i->end() - 1; ++j)
    //        *j = 1;
    for (auto i = 1; i != x_length_ + 1; ++i)
        for (auto j = 1; j != y_length_ + 1; ++j)
            lattice_[i][j] = 1;
}

void Ising2D::sweep(const double & beta, const double & magnetic_b)
{
    for (auto i = 1; i != x_length_ + 1; ++i)
        for (auto j = 1; j != y_length_ + 1; ++j)
        {
            // Compute the energy difference.
            auto s_sum = lattice_[i][j - 1] + lattice_[i][j + 1]
                       + lattice_[i - 1][j] + lattice_[i + 1][j];
            auto energy_difference = 2 * (s_sum + magnetic_b) * lattice_[i][j];
            auto sweep_probability = metropolisFunction(energy_difference, beta);
            //if (randReal_0_1() < sweep_probability)
            if ((static_cast<double>(fastRand()) / RAND_MAX) < sweep_probability)
                lattice_[i][j] *= -1;
        }
}

#endif
