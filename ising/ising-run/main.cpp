#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "ising-core/get-option.h"
#include "ising-core/ising.h"
#include "ising-core/ising-definitions.h"
#include "ising-core/ising-2d.h"
#include "ising-core/ising-parameter.h"

// "Windows.h" should be put after "rapidjson/document.h".
// See https://github.com/Tencent/rapidjson/issues/766
//     http://blog.csdn.net/u011519892/article/details/16985239
#include "ising-core/win-timing.h"

using namespace std;
using namespace ising;
using namespace ising::toolkit;

class EvaluationResult
{
public:
    EvaluationResult() = default;
    EvaluationResult(const Quantity & result, const double & beta, const double & magnetic_h) :
        result_(result), beta_(beta), magnetic_h_(magnetic_h) {}

    friend ostream & operator<<(ostream & os, const EvaluationResult & eval_result)
    {
        const string kSeparator = ",";
        os << eval_result.beta_                   << kSeparator
           << eval_result.magnetic_h_             << kSeparator
           << eval_result.result_.magnetic_dipole << kSeparator
           << eval_result.result_.energy;
        return os;
    }

private:
    Quantity result_;
    double beta_;
    double magnetic_h_;
};

void PrintParameters(const Parameter & param)
{
    cerr << endl
         << "******************************" << endl
         << "Boundary condition: "
         << (param.boundary_condition == kPeriodic ? "periodic" : "free") << endl
         << "Lattice size:       "
         << param.lattice_size.x << "*" << param.lattice_size.y << endl
         << "Iterations:         "
         << param.iterations << endl
         << "******************************" << endl << endl;
}

void PrintProgress(const size_t & total, const size_t & progress)
{
    const size_t     kProgressBarWidth = 80;
    const streamsize kPrecision        = 2;
    const streamsize kDefaultPrecision = cout.precision();
    size_t width = kProgressBarWidth * progress / total;
    // Progress bar.
    cerr << "[" << string(width, '=') << ">" << string(kProgressBarWidth - width, ' ') << "]";
    // Percentage.
    cerr << setw(5 + kPrecision) << setprecision(kPrecision) << fixed
        << 100.0 * progress / total
        << "%\r"
        << skipws << setprecision(kDefaultPrecision) << defaultfloat;
    cerr.flush();
}

template<typename T>
vector<EvaluationResult> Run(vector<T> * eval_list, const Parameter & param)
//vector<EvaluationResult> Run(vector<Ising2D> * eval_list, const Parameter & param)
{
    const auto & beta_list = param.beta_list;
    const auto & h_list    = param.magnetic_h_list;
    size_t beta_list_size = beta_list.size();
    size_t h_list_size    = h_list.size();
    size_t list_size      = beta_list_size * h_list_size;

    vector<EvaluationResult> result_list;
    Timing run_clock;

    // Main running loop.
    // Combine `beta` and `h` into a 1D vector in order to parallelize.
    cerr << "Running..." << endl;
    run_clock.TimingStart();
    ISING_PARALLEL_FOR
    for (auto i = 0; i < list_size; ++i)
    {
        auto beta = beta_list[i % beta_list_size];
        auto h    = h_list[i / beta_list_size];

        auto & cell = (*eval_list)[i];
        cell.Initialize();
        auto result = cell.Evaluate(beta, h,
            param.iterations, param.n_ensemble, param.n_delta);
        result_list.push_back({ result, beta, h });

        PrintProgress(list_size, i + 1);
    }
    run_clock.TimingEnd();
    cerr << endl
         << "Finished!" << endl
         << "Running time: " << run_clock.GetRunningTime() << "s." << endl;

    return result_list;
}

int main(int argc, char * argv[])
{
    GetOption option(argc, argv);
    Parameter parameter(option.Parse('s'));
    parameter.Parse();
    PrintParameters(parameter);

    size_t eval_list_size = parameter.beta_list.size() * parameter.magnetic_h_list.size();
    vector<EvaluationResult> result_list;

    if (parameter.boundary_condition == kPeriodic)
    {
        // Periodic boundary condition.
        vector<Ising2D_PBC> ising_eval_list(eval_list_size, parameter.lattice_size);
        result_list = Run(&ising_eval_list, parameter);
    }
    else
    {
        // Free boundary condition.
        vector<Ising2D_FBC> ising_eval_list(eval_list_size, parameter.lattice_size);
        result_list = Run(&ising_eval_list, parameter);
    }

    for (auto & i : result_list)
        cout << i << endl;

    return 0;
}
