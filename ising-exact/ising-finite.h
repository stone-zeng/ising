#ifndef ISING_EXACT_ISING_FINITE_H_
#define ISING_EXACT_ISING_FINITE_H_

#include <cmath>

#define ISING_INLINE_FLAG inline

typedef unsigned int size_type;

// Math constants.
const double kMathPi   = 3.14159265358979323846;
const double kMathLog2 = 0.69314718055994530942;

// Left-over hyperbolic function.
ISING_INLINE_FLAG double coth(const double & x) { return 1.0 / std::tanh(x); }

ISING_INLINE_FLAG double Gamma(const size_type & q, const size_type & n, const double & k)
{
    if (q == 0)
        return std::log(std::exp(2 * k) * std::tanh(k));
    else
        return std::acosh(std::cosh(2 * k) * coth(2 * k) - std::cos(kMathPi * q / n));
}

// Direct calculation of Y1, ..., Y4 may overflow.
// Use the identity log(Y1 + Y2 + Y3 + Y4) = log(1 + Y2/Y1 + Y3/Y1 + Y4/Y1) + log(Y1).

ISING_INLINE_FLAG double log_Y1(const size_type & n, const size_type & m, const double & k)
{
    double result = 1.0;
    for (auto q = 0; q != n; ++q)
        result += kMathLog2 + std::log(std::cosh(m * Gamma(2 * q + 1, n, k) / 2));
    return result;
}

ISING_INLINE_FLAG double Y2_over_Y1(const size_type & n, const size_type & m, const double & k)
{
    double result = 1.0;
    for (auto q = 0; q != n; ++q)
        result *= std::tanh(m * Gamma(2 * q + 1, n, k) / 2);
    return result;
}

ISING_INLINE_FLAG double Y3_over_Y1(const size_type & n, const size_type & m, const double & k)
{
    double result = 1.0;
    for (auto q = 0; q != n; ++q)
        result *= std::cosh(m * Gamma(2 * q, n, k) / 2) / std::cosh(m * Gamma(2 * q + 1, n, k) / 2);
    return result;
}

ISING_INLINE_FLAG double Y4_over_Y1(const size_type & n, const size_type & m, const double & k)
{
    double result = 1.0;
    for (auto q = 0; q != n; ++q)
        result *= std::sinh(m * Gamma(2 * q, n, k) / 2) / std::cosh(m * Gamma(2 * q + 1, n, k) / 2);
    return result;
}

// `Q` is partition function.
// Use logarithm of partition function since direct calculation can easily overflow.
ISING_INLINE_FLAG double log_Q(const size_type & n, const size_type & m, const double & k)
{
    return -kMathLog2 + n * m / 2 * std::log(2 * std::sinh(2 * k))
        + std::log(1 + Y2_over_Y1(n, m, k) + Y3_over_Y1(n, m, k) + Y4_over_Y1(n, m, k))
        + log_Y1(n, m, k);
}

// `T` is temperature.
ISING_INLINE_FLAG double T_times_log_Q(const size_type & size, const double & T)
{
    return T * log_Q(size, size, 1.0 / T);
}

ISING_INLINE_FLAG double Q_derivative(const size_type & size, const double & T, const double & dT)
{
    // Discrete derivative (1st order).
    return (T_times_log_Q(size, T + dT) - T_times_log_Q(size, T - dT)) / (2 * dT);
}

ISING_INLINE_FLAG double SpecificHeat(const size_type & size, const double & T, const double & dT)
{
    // Discrete derivative (2nd order).
    return T * (Q_derivative(size, T + dT, dT) - Q_derivative(size, T - dT, dT))
        / (2 * dT * size * size);
}

#endif
