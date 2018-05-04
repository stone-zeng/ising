#include <iostream>
#include <string>
#include <vector>

#include "ising-core/exact.h"

#include "ising-core/ising.h"
#include "ising-core/parameter.h"

using namespace std;

ISING_NAMESPACE_BEGIN

IsingExact2D::IsingExact2D(const size_t & size, const double & T) :
    IsingExact2D({ size, size }, T) {}
IsingExact2D::IsingExact2D(const size_t & size, const double & T, const double & dT) :
    IsingExact2D({ size, size }, T, dT) {}
    //n_(size), m_(size), T_(T), dT_(dT) {}
IsingExact2D::IsingExact2D(const LatticeSize & size, const double & T) :
    IsingExact2D(size, T, kDerivativeIncrement * T) {}
IsingExact2D::IsingExact2D(const LatticeSize & size, const double & T, const double & dT) :
    n_(size.x), m_(size.y), T_(T), dT_(dT) {}

// C = dE/dT
//   = d/dT (-d(ln Q)/dk)
//   = T^2 * d^2/dT^2 (ln Q(1/T))
double IsingExact2D::SpecificHeat()
{
    // 2nd order derivative: f''(x) = 1/h^2 * [f(x+h) + f(x-h) - 2*f(x)]
    double c = log_Q(1.0 / (T_ + dT_)) + log_Q(1.0 / (T_ - dT_)) - 2 * log_Q(1.0 / T_);
    return c * pow(T_ / dT_, 2) / (n_ * m_);
}

RunExact::RunExact(const Parameter & param) :
    size_list_(param.lattice_size_list), temperature_list_(param.temperature_list)
{
    result_.resize(temperature_list_.size());
    for (auto & i : result_)
        i.resize(size_list_.size());
}

int RunExact::Run()
{
#ifdef ISING_PARALLEL_FLAG
#pragma omp parallel for
#endif
    for (size_t i = 0; i < temperature_list_.size(); ++i)
    {
        auto T = temperature_list_[i];
        for (auto j = 0; j < size_list_.size(); ++j)
        {
            IsingExact2D e(size_list_[j], T);
            result_[i][j] = e.SpecificHeat();
        }
    }

    PrintFirstRow(cout);
    PrintResult(cout);

    return 0;
}

ISING_NAMESPACE_END
