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
IsingExact2D::IsingExact2D(const LatticeSize & size, const double & T) :
    n_(size.x), m_(size.y), T_(T), dT_(kDerivativeIncrement * T) {}
IsingExact2D::IsingExact2D(const LatticeSize & size, const double & T, const double & dT) :
    n_(size.x), m_(size.y), T_(T), dT_(dT) {}

// E(k) = -dQ(k)/dk
// E(T) = T^2 * d/dT ln Q(1/T)
double IsingExact2D::Energy(const double & T)
{
    // Derivative: f'(x) = 1/(2h) * [f(x+h) - f(x-h)]
    return T * T / (n_ * m_) * (log_Q(1 / (T + dT_)) - log_Q(1 / (T - dT_))) / (2 * dT_);
}

// C(T) = dE(T)/dT
double IsingExact2D::SpecificHeat()
{
    // Derivative: f'(x) = 1/(2h) * [f(x+h) - f(x-h)]
    return (Energy(T_ + dT_) - Energy(T_ - dT_)) / (2 * dT_);
}

Exact::Exact(const Parameter & param) :
    size_list_(param.lattice_size_list), temperature_list_(param.temperature_list)
{
    result_.resize(temperature_list_.size());
    for (auto & i : result_)
        i.resize(size_list_.size());
}

int Exact::Run()
{
    PrintParameter(cerr);
    Evaluate();
    PrintFirstRow(cout);
    PrintResult(cout);

    return 0;
}

void Exact::Evaluate()
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
}

void Exact::PrintParameter(ostream & os)
{
    os << endl << InformationSeparator() << endl;

    os << "* Calculate specific heat for finite size 2D Ising model" << endl
       << "*" << endl
       << "* Parameters:" << endl;

    os << "*   Size list:" << endl
       << "*     ";
    for (auto i : size_list_)
        os << i << " ";
    os << endl;

    os << "*   Temperature range:" << endl
       << "*     "
       << temperature_list_.front() << " -- " << temperature_list_.back() << endl;

    os << InformationSeparator() << endl << endl;
}

void Exact::PrintFirstRow(ostream & os)
{
    for (auto i : size_list_)
        os << "," << i;
    os << endl;
}

void Exact::PrintResult(ostream & os)
{
    for (size_t i = 0; i != temperature_list_.size(); ++i)
    {
        os << temperature_list_[i];
        for (auto j : result_[i])
            os << "," << j;
        os << endl;
    }
}

int RunExact(const Parameter & param)
{
    Exact eval(param);
    return eval.Run();
}

ISING_NAMESPACE_END
