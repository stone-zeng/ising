// Calculate specific heat for finite size 2D Ising model.
// See R.K.Pathria & Paul D. Beale *Statistical Mechanics (3rd ed)* 13.4.A.

#ifndef ISING_CORE_EXACT_H_
#define ISING_CORE_EXACT_H_

#include <cmath>
#include <ostream>
#include <vector>

#include "ising-core/ising.h"
#include "ising-core/parameter.h"

ISING_NAMESPACE_BEGIN

class IsingExact2D
{
public:
    IsingExact2D() = default;
    IsingExact2D(const size_t & size, const double & T);
    IsingExact2D(const size_t & size, const double & T, const double & dT);
    IsingExact2D(const LatticeSize & size, const double & T);
    IsingExact2D(const LatticeSize & size, const double & T, const double & dT);

    double SpecificHeat();

private:
    // Math constants.
    const double kMathPi   = 3.14159265358979323846;
    const double kMathLog2 = 0.69314718055994530942;
    const double kIsingTc  = 2.26918531421302196811; // = 2 / log(1 + sqrt(2))

    const double kDerivativeIncrement = 0.001;

    const size_t n_;
    const size_t m_;
    const double T_;
    const double dT_;

    // Left-over hyperbolic function.
    inline double coth(const double & x) { return 1.0 / std::tanh(x); }

    // [eq. 13.4 (48)] cosh(gamma_q) = cosh^2(2K) / sinh(2K) - cos(pi*q/n), for 0 < q < 2n;
    // [eq. 13.4 (49)] exp(gamma_0)  = exp(2K) * tanh(K).
    inline double gamma(const size_t & q, const double & k)
    {
        if (q == 0)
            return std::log(std::exp(2 * k) * std::tanh(k));
        else
            return std::acosh(std::cosh(2 * k) * coth(2 * k) - std::cos(kMathPi * q / n_));
    }

    // [eq. 13.4 (51)]
    // [a] Y1 = \prod_{q=0}^{n-1} (2cosh(m/2 * gamma_{2q+1}))
    // [b] Y2 = \prod_{q=0}^{n-1} (2sinh(m/2 * gamma_{2q+1}))
    // [c] Y3 = \prod_{q=0}^{n-1} (2cosh(m/2 * gamma_{2q}))
    // [d] Y4 = \prod_{q=0}^{n-1} (2sinh(m/2 * gamma_{2q}))

    // Direct calculation of Y1, ..., Y4 may overflow.
    // Use the identity log(Y1 + Y2 + Y3 + Y4) = log(1 + Y2/Y1 + Y3/Y1 + Y4/Y1) + log(Y1).

    inline double log_Y1(const double & k)
    {
        double result = n_ * kMathLog2;
        for (size_t q = 0; q != n_; ++q)
            result += std::log(std::cosh(m_ * gamma(2 * q + 1, k) / 2));
        return result;
    }

    inline double Y2_over_Y1(const double & k)
    {
        double result = 1.0;
        for (size_t q = 0; q != n_; ++q)
            result *= std::tanh(m_ * gamma(2 * q + 1, k) / 2);
        return result;
    }

    inline double Y3_over_Y1(const double & k)
    {
        double result = 1.0;
        for (size_t q = 0; q != n_; ++q)
            result *= std::cosh(m_ * gamma(2 * q    , k) / 2)
                    / std::cosh(m_ * gamma(2 * q + 1, k) / 2);
        return result;
    }

    inline double Y4_over_Y1(const double & k)
    {
        double result = 1.0;
        for (size_t q = 0; q != n_; ++q)
            result *= std::sinh(m_ * gamma(2 * q    , k) / 2)
                    / std::cosh(m_ * gamma(2 * q + 1, k) / 2);
        return result;
    }

    // `Q` is the partition function.
    // [eq. 13.4 (50)] Q_{nm} (K) = 1/2 * (2sinh(2K))^(nm/2) * (Y1+Y2+Y3+Y4)
    // Use logarithm of partition function since direct calculation can easily overflow.
    inline double log_Q(const double & k)
    {
        return -kMathLog2 + n_ * m_ / 2 * std::log(2 * std::sinh(2 * k))
            + log_Y1(k) + std::log(1 + Y2_over_Y1(k) + Y3_over_Y1(k) + Y4_over_Y1(k));
    }
};

class RunExact
{
public:
    RunExact() = default;
    RunExact(const Parameter & param);

    int Run();

private:
    typedef std::vector<double> Result;

    std::vector<LatticeSize> size_list_;
    std::vector<double>      temperature_list_;
    std::vector<Result>      result_;

    inline void PrintFirstRow(std::ostream & os)
    {
        for (auto i : size_list_)
            os << "," << i.x << "*" << i.y;
        os << std::endl;
    }

    inline void PrintResult(std::ostream & os)
    {
        for (auto i = 0; i != temperature_list_.size(); ++i)
        {
            os << temperature_list_[i];
            for (auto j : result_[i])
                os << "," << j;
            os << std::endl;
        }
    }
};

ISING_NAMESPACE_END

#endif
