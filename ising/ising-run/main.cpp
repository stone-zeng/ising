/*
#define DATA_GENERATION_ON

#ifdef DATA_GENERATION_ON
#include <iostream>
#include <vector>
#include "ising-2d.h"
#include "win-timing.h"

using namespace std;

const double c_beta_critical = 0.4406867935;

void outputVector(const vector<int> & v)
{
    for (auto i = v.begin(); i != v.end() - 1; ++i)
        cout << *i << ',';
    cout << *(v.end() - 1) << endl;
}

int main(int argc, char * argv[])
{
    size_t size    = stoi(argv[1]);
    size_t scale   = stoi(argv[2]);
    size_t steps   = stoi(argv[3]);
    size_t samples = stoi(argv[4]);

    cerr << "0% ";

    Timing clock;

    clock.TimingStart();
#pragma omp parallel for
    for (auto i = 0; i < samples; ++i)
    {
        Ising2D s{ size, size };
        s.Evaluate(c_beta_critical, 0.0, steps, 0);
        auto result = s.Renormalize(scale, scale);
        outputVector(result);

        if (samples < 10) { cerr << "=="; }
        else
            if ((i + 1) % (samples / 10) == 0)
                cerr << "==";
    }
    clock.TimingEnd();

    cerr << "Computation time: " << clock.GetRunningTime() << "s." << endl;

    return 0;
}
#endif

#ifndef DATA_GENERATION_ON
#include <array>
#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <omp.h>
#include "ising-2d.h"
#include "win-timing.h"

#define $ENSEMBLE_NUMBER    120
#define $ENSEMBLE_DELTA     4
#define $REPEAT_NUMBER      100

using namespace std;

Quantity eval(const size_t & size, const size_t & steps, const double & temperature)
{
    Ising2D s{ size,size };
    // Set magnetic_b = 0.
    return s.Evaluate(1.0 / temperature, 0.0, steps, $ENSEMBLE_NUMBER, $ENSEMBLE_DELTA);
}

struct EvalCell
{
    double temperature;
    array<Quantity, $REPEAT_NUMBER> quantity;

    friend ostream & operator<<(ostream & os, const EvalCell & cell)
    {
        os << cell.temperature;
        for (auto i : cell.quantity)
            os << "," << i.magnetic_dipole << "," << i.energy;
        return os;
    }
};

int main(int argc, char * argv[])
{
    auto size = stoi(argv[1]);
    auto steps = stoi(argv[2]);
    auto t_begin = stod(argv[3]);
    auto t_end = stod(argv[4]);
    auto t_num = stoi(argv[5]);

    vector<EvalCell> result_arr(t_num + 1);
    for (auto i = 0; i < t_num + 1; ++i)
        result_arr[i].temperature = static_cast<double>(i) / static_cast<double>(t_num)
            * (t_end - t_begin) + t_begin;

    cerr << "0% ";

    Timing clock;

    clock.TimingStart();
#pragma omp parallel for
    for (auto i = 0; i < t_num + 1; ++i)
    {
        for(auto & j : result_arr[i].quantity)
            j = eval(size, steps, result_arr[i].temperature);
        if (t_num < 10) { cerr << "=="; }
        else
            if ((i + 1) % (t_num / 10) == 0)
                cerr << "==";
    }
    clock.TimingEnd();

    cerr << "> 100%  Finished!" << endl
         << "Computation time: " << clock.GetRunningTime() << "s." << endl;

    for (auto i = result_arr.begin(); i != result_arr.end(); ++i)
        cout << *i << endl;

    return 0;
}

#endif
*/

//#include "ising-2d.h"
//using namespace std;
//
//int main()
//{
//    return 0;
//}

#include <fstream>
#include <iostream>

#include "ising-core/getopt.h"
#include "ising-core/ising-2d.h"
#include "ising-core/ising-definitions.h"

using namespace std;
using namespace ising;
using namespace ising::toolkit;

int main(int argc, char * argv[])
{
    GetOption option(argc, argv);
    auto settings_path_str = option.Parse('s');

    size_t x_length = 10;
    size_t y_length = 10;
    double beta = 1;
    double magnet = 0;
    size_t iteration = 100;
    size_t n_ensemble = 10;

    Ising2D_FBC s(x_length, y_length);
    s.Initialize();
    Quantity result;

    for (auto t = 0.01; t < 4; t += 0.01)
    {
        result = s.Evaluate(1 / t, magnet, iteration, n_ensemble);
        cout << t << ", " << result.magnetic_dipole << ", " << result.energy << endl;
    }
}
