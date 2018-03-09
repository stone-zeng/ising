#include <iostream>
#include <vector>

#include "ising-core/get-option.h"
#include "ising-core/ising.h"
#include "ising-core/ising-definitions.h"
#include "ising-core/ising-2d.h"
#include "ising-core/ising-parameter.h"

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

template<typename T>
vector<EvaluationResult> Run(vector<T> * eval_list, const Parameter & param)
{
    const auto & beta_list = param.beta_list;
    const auto & h_list    = param.magnetic_h_list;
    vector<EvaluationResult> result_list;

    // Main running loop.
    ISING_PARALLEL_FOR
    for (auto i = 0; i < beta_list.size() * h_list.size(); ++i)
    {
        auto beta = beta_list[i % beta_list.size()];
        auto h    = h_list[i / beta_list.size()];

        (*eval_list)[i].Initialize();
        auto result = (*eval_list)[i].Evaluate(beta, h,
            param.iterations, param.n_ensemble, param.n_delta);
        result_list.push_back({ result, beta, h });
    }
    return result_list;
}

int main(int argc, char * argv[])
{
    GetOption option(argc, argv);
    Parameter parameter(option.Parse('s'));
    parameter.Parse();

    size_t eval_list_size = parameter.beta_list.size() * parameter.magnetic_h_list.size();
    vector<EvaluationResult> result_list;

    cerr << "******************************" << endl
         << "Boundary condition: "
         << (parameter.boundary_condition == kPeriodic ? "periodic" : "free") << endl
         << "Lattice size:       "
         << parameter.lattice_size.x << "*" << parameter.lattice_size.y << endl
         << "Iterations:         "
         << parameter.iterations << endl
         << "******************************" << endl << endl;

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
