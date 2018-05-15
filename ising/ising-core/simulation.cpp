#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <include/argagg/argagg.hpp>

#include "ising-core/simulation.h"

#include "ising-core/ising.h"
#include "ising-core/ising-2d.h"
#include "ising-core/parameter.h"

// "Windows.h" should be put after "rapidjson/document.h".
// See https://github.com/Tencent/rapidjson/issues/766
//     http://blog.csdn.net/u011519892/article/details/16985239
#include "ising-core/timing.h"

using namespace std;

ISING_NAMESPACE_BEGIN

EvalCell::EvalCell(const vector<size_t> & size_list) :
    eval_list_(size_list.begin(), size_list.end()) {}

int EvalCell::Run(const double & temperature, const double & magnetic_h,
    const size_t & iterations, const size_t & n_ensemble, const size_t & n_delta)
{
    for (auto & cell : eval_list_)
    {
        cell.Initialize();
        auto result = cell.Evaluate(1.0 / temperature, magnetic_h, iterations, n_ensemble, n_delta);
        result_list_.push_back(result);
    }
}

Simulation::Simulation(const Parameter & param) :
    size_list_(param.lattice_size_list),
    temperature_list_(param.temperature_list),
    magnetic_h_list_(param.magnetic_h_list),
    iterations_(param.iterations),
    n_ensemble_(param.n_ensemble),
    n_delta_(param.n_delta),
    repetitions_(param.repetitions),
    eval_cell_num_(temperature_list_.size() * magnetic_h_list_.size()),
    eval_list_(size_list_.size(), vector<EvalCell>(eval_cell_num_, size_list_)) {}

int Simulation::Run()
{
    for (auto v : eval_list_)
    {
#ifdef ISING_PARALLEL
#pragma omp parallel for
#endif
        for (size_t i = 0; i < eval_cell_num_; ++i)
        {
            v[i];
        }
    }
}

template<typename T>
ResultList Run(vector<T> * eval_list, const Parameter & param)
{
    const auto & t_list = param.temperature_list;
    const auto & h_list = param.magnetic_h_list;
    size_t t_list_size  = t_list.size();
    size_t h_list_size  = h_list.size();
    size_t list_size    = t_list_size * h_list_size;

    ResultList result_list;
    result_list.resize(list_size);

    Timing run_clock;

    // Main running loop.
    // Combine `beta` and `h` into a 1D vector in order to parallelize.
    cerr << "Running..." << endl;
    run_clock.TimingBegin();
#ifdef ISING_PARALLEL
#pragma omp parallel for
#endif
    for (auto i = 0; i < list_size; ++i)
    {
        auto temperature = t_list[i % t_list_size];
        auto h = h_list[i / t_list_size];
        auto & cell = (*eval_list)[i];

        // Repeat on a single parameter set.
        for (auto j = 0; j < param.repetitions; ++j)
        {
            cell.Initialize();
            auto result = cell.Evaluate(1.0 / temperature, h,
                param.iterations, param.n_ensemble, param.n_delta);
            result_list[i].push_back({ result, 1.0 / temperature, h });
        }

        PrintProgress(list_size, i + 1);
    }
    run_clock.TimingEnd();
    cerr << endl
         << "Finished!" << endl
         << "Running time: " << run_clock.GetRunningTime() << "s." << endl;

    return result_list;
}

void Simulation::PrintParameters(std::ostream & os, const Parameter & param)
{
    os << endl << InformationSeparator() << endl;

    os << "Boundary condition: "
       << (param.boundary_condition == kPeriodic ? "Periodic" : "Free") << endl
       << "Lattice size:       "
       << param.lattice_size << endl
       << "Iterations:         "
       << param.iterations << endl
       << "Repetitions:        "
       << param.repetitions << endl
       << "Parallelization:    "
#ifdef ISING_PARALLEL
       << "On" << endl
#else
       << "Off" << endl
#endif
       << "Fast exp():         "
#ifdef ISING_FAST_EXP
       << "On" << endl
#else
       << "Off" << endl
#endif
       << InformationSeparator() << endl << endl;
}

//void Simulation::PrintResults(ostream & os, const ResultList & result_list)
//{
//}

// The results are in CSV format.
//void Simulation::PrintResults(ostream & os, const Simulation::ResultList & result_list)
//{
//    // Header.
//    os << "T,H";
//    for (auto i = 0; i != result_list[0].size(); ++i)
//        os << ",M,M.abs,M.sq,E,E.sq";
//    os << endl;
//
//    // Main body.
//    for (auto i : result_list)
//    {
//        os << i.begin()->Parameters() << ",";
//        for (auto j = i.begin(); j != i.end() - 1; ++j)
//            os << j->Result() << ",";
//        os << (i.end() - 1)->Result() << endl;
//    }
//}

void Simulation::PrintProgress(const size_t & total, const size_t & progress)
{
    const size_t     kProgressBarWidth = 40;
    const streamsize kPrecision        = 2;
    const streamsize kDefaultPrecision = cerr.precision();
#ifndef ISING_PARALLEL
    size_t width = kProgressBarWidth * progress / total;
    // Progress bar.
    cerr << "[" << string(width, '=') << ">" << string(kProgressBarWidth - width, ' ') << "]";
    // Percentage.
    cerr << setw(5 + kPrecision) << setprecision(kPrecision) << fixed
         << 100.0 * progress / total
         << "%\r"
         << skipws << setprecision(kDefaultPrecision) << defaultfloat;
    cerr.flush();
#else
    if (total < kProgressBarWidth)
        cerr << "=";
    else
        if ((progress + 1) % (total / kProgressBarWidth) == 0)
            cerr << "=";
#endif
}

int RunSimulation(const Parameter & param)
{
    Simulation eval(param);
    return eval.Run();
}

ISING_NAMESPACE_END








class EvaluationResult
{
public:
    EvaluationResult() = default;
    EvaluationResult(const Observable & result, const double & beta, const double & magnetic_h) :
        result_(result), beta_(beta), magnetic_h_(magnetic_h) {}

    string Parameters()
    {
        // Use temperature here.
        return to_string(1 / beta_) + kSeparator + to_string(magnetic_h_);
    }
    string Result()
    {
        return to_string(result_.magnetic_dipole)        + kSeparator
             + to_string(result_.magnetic_dipole_abs)    + kSeparator
             + to_string(result_.magnetic_dipole_square) + kSeparator
             + to_string(result_.energy)                 + kSeparator
             + to_string(result_.energy_square);
    }

private:
    Observable result_;
    double beta_;
    double magnetic_h_;

    const string kSeparator = ",";
};

template<typename T>
ResultList Run(vector<T> * eval_list, const Parameter & param)
{
    const auto & t_list = param.temperature_list;
    const auto & h_list = param.magnetic_h_list;
    size_t t_list_size  = t_list.size();
    size_t h_list_size  = h_list.size();
    size_t list_size    = t_list_size * h_list_size;

    ResultList result_list;
    result_list.resize(list_size);

    Timing run_clock;

    // Main running loop.
    // Combine `beta` and `h` into a 1D vector in order to parallelize.
    cerr << "Running..." << endl;
    run_clock.TimingBegin();
#ifdef ISING_PARALLEL
#pragma omp parallel for
#endif
    for (auto i = 0; i < list_size; ++i)
    {
        auto temperature = t_list[i % t_list_size];
        auto h = h_list[i / t_list_size];
        auto & cell = (*eval_list)[i];

        // Repeat on a single parameter set.
        for (auto j = 0; j < param.repetitions; ++j)
        {
            cell.Initialize();
            auto result = cell.Evaluate(1.0 / temperature, h,
                param.iterations, param.n_ensemble, param.n_delta);
            result_list[i].push_back({ result, 1.0 / temperature, h });
        }

        PrintProgress(list_size, i + 1);
    }
    run_clock.TimingEnd();
    cerr << endl
         << "Finished!" << endl
         << "Running time: " << run_clock.GetRunningTime() << "s." << endl;

    return result_list;
}

int main0(int argc, char * argv[])
{
    argagg::parser arg_parser
        { {
            { "settings", { "--settings", "-s" }, "", 1 },
            { "dumped",   { "--dumped",   "-d" }, "", 0 }
        } };
    argagg::parser_results args = arg_parser.parse(argc, argv);

    Parameter parameter;
    parameter.ReadFromFile(args["settings"].as<string>(""));
    parameter.Parse();
    PrintParameters(parameter);

    size_t eval_list_size = parameter.temperature_list.size() * parameter.magnetic_h_list.size();
    ResultList result_list;

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

    PrintResults(cout, result_list);

    return 0;
}
