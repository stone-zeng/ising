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

/*
class Ising2D
{
public:
    Ising2D();
    Ising2D(const size_t & x_length, const size_t & y_length);
    ~Ising2D();

    Quantity evaluate(const double & beta, const double & magnetic_b, const size_t & steps,
        const size_t & n_ensemble, const size_t & n_delta = 1);
    Quantity analysis(const double & magnetic_b);
    std::vector<int> renormalize(const size_t & x_scale, const size_t & y_scale);
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

std::vector<int> Ising2D::renormalize(const size_t & x_scale, const size_t & y_scale)
{
    size_t x_length_renormalized = x_length_ / x_scale;
    size_t y_length_renormalized = y_length_ / y_scale;

    std::vector<int> result;
    result.resize(x_length_renormalized * y_length_renormalized);

    std::vector<std::vector<int>> local_lattice;
    local_lattice.resize(x_scale);
    for (auto i = local_lattice.begin(); i != local_lattice.end(); ++i)
        i->resize(y_scale);

    for (auto i = 0; i != x_length_renormalized; ++i)
        for (auto j = 0; j != y_length_renormalized; ++j)
        {
            std::vector<int> local_lattice;
            local_lattice.resize(x_scale * y_scale);
            for (auto k = 0; k != x_scale; ++k)
                for (auto l = 0; l != y_scale; ++l)
                    // Use "+1" to skip the boundary.
                    local_lattice[y_scale * k + l]
                        = lattice_[x_scale * i + k + 1][y_scale * j + l + 1];

            for (auto t = 0; t != x_scale * y_scale; ++t)
                result[y_length_renormalized * i + j]
                    += (local_lattice[t] + 1) / 2 * static_cast<int>(std::pow(2, t));
        }

    return result;
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
*/


class Ising2D
{
public:
    Ising2D() = default;
    Ising2D(const size_t & x_length, const size_t & y_length)
    {
        x_length_ = x_length;
        y_length_ = y_length;
        initialize();
    }

    ~Ising2D() = default;

    //Quantity evaluate(const double & beta, const double & magnetic_b, const size_t & steps,
    //    const size_t & n_ensemble, const size_t & n_delta = 1);
    //Quantity analysis(const double & magnetic_b);
    //std::vector<int> renormalize(const size_t & x_scale, const size_t & y_scale);

    inline void show()
    {
        for (auto i = lattice_.begin(); i != lattice_.end(); ++i)
        {
            for (auto j = i->begin(); j != i->end(); ++j)
                std::cout << *j << " ";
            std::cout << std::endl;
        }
    }

protected:
    size_t x_length_;
    size_t y_length_;

    std::vector<std::vector<int>> lattice_;

    virtual void initialize() {}
    virtual void sweep(const double & beta, const double & magnetic_b) {}

    inline bool isFlip(const int & spin_sum, const int & spin_value,
        const double & magnetic_b, const double & beta)
    {
        auto energy_difference = 2 * (spin_sum + magnetic_b) * spin_value;
        auto flip_probability = metropolisFunction(energy_difference, beta);
        return (static_cast<double>(fastRand()) / RAND_MAX) < flip_probability;
    }
    inline double metropolisFunction(const double & energy, const double & beta)
    {
        auto boltzmann_probability = std::exp(-beta * energy);
        return boltzmann_probability < 1.0 ? boltzmann_probability : 1.0;
    }
};

// Periodic boundary condition.
class Ising2D_PBC : public Ising2D
{
public:
    Ising2D_PBC() = default;
private:
    void initialize() override;
    void sweep(const double & beta, const double & magnetic_b) override;

    inline size_t xPlusOne (const size_t & x) { return (x == x_length_ - 1 ? 0 : x + 1); }
    inline size_t xMinusOne(const size_t & x) { return (x == 0 ? x_length_ - 1 : x - 1); }
    inline size_t yPlusOne (const size_t & y) { return (y == y_length_ - 1 ? 0 : y + 1); }
    inline size_t yMinusOne(const size_t & y) { return (y == 0 ? y_length_ - 1 : y - 1); }
};

// Free boundary condition (with zero padding).
class Ising2D_FBC : public Ising2D
{
public:
    Ising2D_FBC() = default;
private:
    void initialize() override;
    void sweep(const double & beta, const double & magnetic_b) override;
};

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
            if (isFlip(spin_sum, lattice_[i][j], magnetic_b, beta))
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
            if (isFlip(spin_sum, lattice_[i][j], magnetic_b, beta))
                lattice_[i][j] *= -1;
        }
}

#endif
