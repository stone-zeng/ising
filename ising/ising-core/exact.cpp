#include <iostream>
#include <string>
#include <vector>

#include "ising-core/ising.h"
#include "ising-core/exact.h"
#include "ising-core/ising-definitions.h"
#include "ising-core/ising-parameter.h"

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

ISING_NAMESPACE_END


//ISING_EXACT_NAMESPACE_BEGIN
//
//typedef vector<size_type> SizeVector;
//typedef vector<double>    TemperatureVector;
//typedef vector<double>    Result;
//typedef vector<Result>    ResultVector;
//
//SizeVector GetSizeVector(const size_type & max)
//{
//    SizeVector v;
//    for (size_type i = 1; i != max + 1; ++i)
//        v.push_back(static_cast<size_type>(pow(2, i)));
//    return v;
//}
//
//TemperatureVector GetTemperatureVector(const double & t_begin, const double & t_end, const double & dt)
//{
//    TemperatureVector v;
//    for (auto t = t_begin; t <= t_end + dt; t += dt)
//        v.push_back(t);
//    return v;
//}
//
//void PrintFirstRow(ostream & os, const SizeVector & v)
//{
//    for (auto i : v)
//        os << "," << i;
//    os << endl;
//}
//
//void PrintResult(ostream & os, const TemperatureVector & T_v, const ResultVector & result_v)
//{
//    for (auto i = 0; i != T_v.size(); ++i)
//    {
//        os << T_v[i];
//        for (auto j : result_v[i])
//            os << "," << j;
//        os << endl;
//    }
//}
//
//ISING_EXACT_NAMESPACE_END
//
//ISING_NAMESPACE_BEGIN
//
//int RunExact(int argc, char * argv[], const Parameter & param)
//{
//    const double kEpsilon = 1e-4;
//
//    const double T_begin = 1.0;
//    const double T_end   = 4.0;
//
//    exact::size_type size_max = stoi(argv[1]);
//    double dT = stod(argv[2]);
//
//    auto size_vec = exact::GetSizeVector(size_max);
//    auto T_vec    = exact::GetTemperatureVector(T_begin, T_end, dT);
//
//    exact::ResultVector result_vec(T_vec.size());
//    for (auto & i : result_vec)
//        i.resize(size_vec.size());
//
//#ifdef ISING_PARALLEL_FLAG
//#pragma omp parallel for
//#endif
//    for (auto T_idx = 0; T_idx < T_vec.size(); ++T_idx)
//    {
//        auto T = T_vec[T_idx];
//        for (auto size_idx = 0; size_idx < size_vec.size(); ++size_idx)
//            result_vec[T_idx][size_idx] = exact::SpecificHeat(size_vec[size_idx], T, kEpsilon);
//    }
//
//    exact::PrintFirstRow(cout, size_vec);
//    exact::PrintResult(cout, T_vec, result_vec);
//
//    return 0;
//}
//
//ISING_NAMESPACE_END
