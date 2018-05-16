#include "core/simulation.h"

#include <iostream>
#include <string>
#include <vector>
#include <include/rapidjson/document.h>
#include <include/rapidjson/writer.h>

#include "core/info.h"
#include "core/ising.h"
#include "core/ising-2d.h"
#include "core/parameter.h"

// "Windows.h" should be put after "rapidjson/document.h".
// See https://github.com/Tencent/rapidjson/issues/766
//     http://blog.csdn.net/u011519892/article/details/16985239
#include "core/timing.h"

using namespace std;
using namespace ising::toolkit;

ISING_NAMESPACE_BEGIN

SimulationUnit::SimulationUnit(const size_t & repetitions, const size_t & lattice_size) :
    eval_list_(repetitions, lattice_size) {}

void SimulationUnit::Run(const double & temperature, const double & magnetic_h,
    const size_t & iterations, const size_t & n_ensemble, const size_t & n_delta)
{
    Observable result;
    for (auto & cell : eval_list_)
    {
        cell.Initialize();
        result = cell.Evaluate(1.0 / temperature, magnetic_h, iterations, n_ensemble, n_delta);
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
    size_list_size_(size_list_.size()),
    eval_cell_num_(temperature_list_.size() * magnetic_h_list_.size()),
    // Initialize `eval_list_` and `result_list_` with correct dimensions.
    eval_list_(size_list_size_, vector<SimulationUnit>(eval_cell_num_)),
    result_list_(size_list_size_,
        vector<vector<Observable>>(eval_cell_num_, vector<Observable>(repetitions_)))
{
    // Initialize `eval_list_` with correct `size` parameter.
    for (size_t i = 0; i != size_list_size_; ++i)
        for (size_t j = 0; j != eval_cell_num_; ++j)
            eval_list_[i][j] = SimulationUnit(repetitions_, size_list_[i]);
}

int Simulation::Run()
{
    PrintParameters(cerr);
    Simulate();
    PrintResults(cout);

    return 0;
}

void Simulation::Simulate()
{
    const auto kTemperatureListSize = temperature_list_.size();
    Timing run_clock;

    for (size_t i = 0; i != size_list_size_; ++i)
    {
        cerr << "Running on size " << size_list_[i] << "..." << endl;

        run_clock.TimingBegin();
#ifdef ISING_PARALLEL
#pragma omp parallel for
#endif
        // OpenMP for need signed integer.
        for (int j = 0; j < eval_cell_num_; ++j)
        {
            auto & eval = eval_list_[i][j];
            auto t = temperature_list_[j % kTemperatureListSize];
            auto h = magnetic_h_list_[j / kTemperatureListSize];
            eval.Run(t, h, iterations_, n_ensemble_, n_delta_);
            result_list_[i][j] = eval.Result();

            PrintProgress(eval_cell_num_, j + 1);
        }
        run_clock.TimingEnd();
        cerr << endl
             << "Running time: " << run_clock.GetRunningTime() << "s." << endl << endl;
    }
    
    cerr << "Finished!" << endl;
}

void Simulation::PrintParameters(std::ostream & os)
{
    os << endl << InformationSeparator() << endl;

    os << "* Analyze critical behavior with Monte Carlo algorithm" << endl
       << "*" << endl
       << "* Parameters:" << endl;

    os << "*   Boundary condition: "
       // TODO: use only Periodic boundary condition now.
       // << (param.boundary_condition == kPeriodic ? "Periodic" : "Free") << endl
       << "Periodic" << endl;

    os << "*   Size list:" << endl
       << "*     ";
    for (auto i : size_list_)
        os << i << " ";
    os << endl;

    os << "*   Temperature range:" << endl
       << "*     "
       << temperature_list_.front() << " -- " << temperature_list_.back() << endl;

    os << "*   External magnetic field range:" << endl
       << "*     "
       << magnetic_h_list_.front() << " -- " << magnetic_h_list_.back() << endl;

    os << "*   Iterations:         "
       << iterations_ << endl
       << "*   Repetitions:        "
       << repetitions_ << endl
       << "*   Parallelization:    "
#ifdef ISING_PARALLEL
       << "On" << endl
#else
       << "Off" << endl
#endif
       << "*   Fast exp():         "
#ifdef ISING_FAST_EXP
       << "On" << endl
#else
       << "Off" << endl
#endif
       << InformationSeparator() << endl << endl;
}

void Simulation::PrintResults(ostream & os)
{
    const auto kTemperatureListSize = temperature_list_.size();

    rapidjson::Document doc(rapidjson::Type::kArrayType);
    auto & doc_allocator = doc.GetAllocator();

    for (size_t i = 0; i != size_list_size_; ++i)
        for (size_t j = 0; j != eval_cell_num_; ++j)
        {
            rapidjson::Value cell_val(rapidjson::Type::kObjectType);

            // Parameters.
            cell_val.AddMember("size", size_list_[i], doc_allocator);
            cell_val.AddMember("temperature",
                temperature_list_[j % kTemperatureListSize], doc_allocator);
            cell_val.AddMember("externalMagneticField ",
                magnetic_h_list_[j / kTemperatureListSize], doc_allocator);

            // Simulation results (observables)
            rapidjson::Value magnetic_dipole(rapidjson::Type::kArrayType);
            rapidjson::Value magnetic_dipole_abs(rapidjson::Type::kArrayType);
            rapidjson::Value magnetic_dipole_square(rapidjson::Type::kArrayType);
            rapidjson::Value energy(rapidjson::Type::kArrayType);
            rapidjson::Value energy_square(rapidjson::Type::kArrayType);

            for (auto result : result_list_[i][j])
            {
                magnetic_dipole.PushBack(result.magnetic_dipole, doc_allocator);
                magnetic_dipole_abs.PushBack(result.magnetic_dipole_abs, doc_allocator);
                magnetic_dipole_square.PushBack(result.magnetic_dipole_square, doc_allocator);
                energy.PushBack(result.energy, doc_allocator);
                energy_square.PushBack(result.energy_square, doc_allocator);
            }

            cell_val.AddMember("magneticDipole", magnetic_dipole, doc_allocator);
            cell_val.AddMember("magneticDipole.Abs", magnetic_dipole_abs, doc_allocator);
            cell_val.AddMember("magneticDipole.Square", magnetic_dipole_square, doc_allocator);
            cell_val.AddMember("energy", energy, doc_allocator);
            cell_val.AddMember("energy.Square", energy_square, doc_allocator);

            // Add to the outer array.
            doc.PushBack(cell_val, doc_allocator);
        }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    string json_str(buffer.GetString());
    os << json_str << endl;
}

int RunSimulation(const Parameter & param)
{
    Simulation eval(param);
    return eval.Run();
}

ISING_NAMESPACE_END
