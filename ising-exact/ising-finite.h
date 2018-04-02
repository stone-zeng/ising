#ifndef ISING_EXACT_ISING_FINITE_H_
#define ISING_EXACT_ISING_FINITE_H_

#include <cmath>

#define ISING_INLINE_FLAG inline

typedef unsigned int size_type;

const double kMathPi = 3.14159265358979323846;

ISING_INLINE_FLAG double coth(const double & x) { return 1.0 / std::tanh(x); }

ISING_INLINE_FLAG double Gamma(const size_type & q, const size_type & n, const double & k)
{
    if (q == 0)
        return std::log(std::exp(2 * k) * std::tanh(k));
    else
        return std::acosh(std::cosh(2 * k) * coth(2 * k) - std::cos(kMathPi * q / n));
}

ISING_INLINE_FLAG double Y1(const size_type & n, const size_type & m, const double & k)
{
    double result = 1.0;
    for (auto q = 0; q != n; ++q)
        result *= 2 * std::cosh(m * Gamma(2 * q + 1, n, k) / 2);
    return result;
}

ISING_INLINE_FLAG double Y2(const size_type & n, const size_type & m, const double & k)
{
    double result = 1.0;
    for (auto q = 0; q != n; ++q)
        result *= 2 * std::sinh(m * Gamma(2 * q + 1, n, k) / 2);
    return result;
}

ISING_INLINE_FLAG double Y3(const size_type & n, const size_type & m, const double & k)
{
    double result = 1.0;
    for (auto q = 0; q != n; ++q)
        result *= 2 * std::cosh(m * Gamma(2 * q, n, k) / 2);
    return result;
}

ISING_INLINE_FLAG double Y4(const size_type & n, const size_type & m, const double & k)
{
    double result = 1.0;
    for (auto q = 0; q != n; ++q)
        result *= 2 * std::sinh(m * Gamma(2 * q, n, k) / 2);
    return result;
}

ISING_INLINE_FLAG double PartitionFunction(const size_type & n, const size_type & m,
                                           const double & k)
{
    return std::pow(2 * std::sinh(2 * k), n * m / 2) / 2
        * (Y1(n, m, k) + Y2(n, m, k) + Y3(n, m, k) + Y4(n, m, k));
}

ISING_INLINE_FLAG double _SpecificHeatAuxI(const size_type & size, const double & T)
{
    // `T` = temperature
    return T * std::log(PartitionFunction(size, size, 1.0 / T));
}

ISING_INLINE_FLAG double _SpecificHeatAuxII(const size_type & size, const double & T,
                                            const double & dT)
{
    // Discrete derivative (1st order).
    return (_SpecificHeatAuxI(size, T + dT) - _SpecificHeatAuxI(size, T - dT)) / (2 * dT);
}

ISING_INLINE_FLAG double SpecificHeat(const size_type & size, const double & T, const double & dT)
{
    // Discrete derivative (2nd order).
    return T * (_SpecificHeatAuxII(size, T + dT, dT) - _SpecificHeatAuxII(size, T - dT, dT))
        / (2 * dT * size * size);
}

#endif
