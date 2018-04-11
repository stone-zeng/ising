#include <iostream>
#include <string>
#include <vector>
#include "exact.h"

using namespace std;

typedef vector<size_type> SizeVector;
typedef vector<double>    TemperatureVector;
typedef vector<double>    Result;
typedef vector<Result>    ResultVector;

SizeVector GetSizeVector(const size_type & max)
{
    SizeVector v;
    for (size_type i = 1; i != max + 1; ++i)
        v.push_back(static_cast<size_type>(pow(2, i)));
    return v;
}

TemperatureVector GetTemperatureVector(const double & t_begin, const double & t_end, const double & dt)
{
    TemperatureVector v;
    for (auto t = t_begin; t <= t_end + dt; t += dt)
        v.push_back(t);
    return v;
}

void PrintFirstRow(ostream & os, const SizeVector & v)
{
    for (auto i : v)
        os << "," << i;
    os << endl;
}

void PrintResult(ostream & os, const TemperatureVector & T_v, const ResultVector & result_v)
{
    for (auto i = 0; i != T_v.size(); ++i)
    {
        os << T_v[i];
        for (auto j : result_v[i])
            os << "," << j;
        os << endl;
    }
}

int main(int argc, char * argv[])
{
    const double kEpsilon = 1e-4;

    const double T_begin = 1.0;
    const double T_end   = 4.0;

    size_type size_max = stoi(argv[1]);
    double    dT       = stod(argv[2]);

    auto size_vec = GetSizeVector(size_max);
    auto T_vec    = GetTemperatureVector(T_begin, T_end, dT);

    ResultVector result_vec(T_vec.size());
    for (auto & i : result_vec)
        i.resize(size_vec.size());

#ifdef ISING_PARALLEL_FLAG
#pragma omp parallel for
#endif
    for (auto T_idx = 0; T_idx < T_vec.size(); ++T_idx)
    {
        auto T = T_vec[T_idx];
        for (auto size_idx = 0; size_idx < size_vec.size(); ++size_idx)
            result_vec[T_idx][size_idx] = SpecificHeat(size_vec[size_idx], T, kEpsilon);
    }

    PrintFirstRow(cout, size_vec);
    PrintResult(cout, T_vec, result_vec);

    return 0;
}
