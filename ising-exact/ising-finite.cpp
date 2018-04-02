#include <iostream>
#include <string>
#include <vector>
#include "ising-finite.h"

using namespace std;

int main(int argc, char * argv[])
{
    size_type size = stoi(argv[1]);
    double epsilon = stod(argv[2]);

    vector<double> T_vec;
    for (auto i = 1.0; i <= 4.0; i += 0.1)
        T_vec.push_back(i);

    for (auto T : T_vec)
        cout << T << "," << SpecificHeat(size, T, epsilon) << endl;

    return 0;
}
