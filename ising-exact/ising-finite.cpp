#include <iostream>
#include <string>
#include <vector>
#include "ising-finite.h"

using namespace std;

typedef vector<double> Result;

int main(int argc, char * argv[])
{
    const double kEpsilon = 1e-4;

    auto size_arr = { 4, 8, 16, 32, 64, 128, 256, 512, 1024 };

    vector<double> T_vec;
    for (auto i = 1.0; i <= 4.0; i += 0.001)
        T_vec.push_back(i);

    for (auto T : T_vec)
    {
        cout << T << ",";
        for (auto i = size_arr.begin(); i != size_arr.end() - 1; ++i)
            cout << SpecificHeat(*i, T, kEpsilon) << ",";
        cout << SpecificHeat(*(size_arr.end() - 1), T, kEpsilon) << endl;
    }

    return 0;
}
