#include "core/lattice-data.h"

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

LatticeDataUnit::LatticeDataUnit(const size_t & repetitions, const size_t & lattice_size) :
    eval_list_(repetitions, lattice_size) {}

void LatticeDataUnit::Run(const double & temperature, const double & magnetic_h,
    const size_t & iterations)
{
    Lattice2D result;
    for (auto & cell : eval_list_)
    {
        cell.Initialize();
        result = cell.EvaluateLatticeData(1.0 / temperature, magnetic_h, iterations).lattice_data;
        result_list_.push_back(result);
    }
}

LatticeData::LatticeData(const Parameter & param) :
    size_list_(param.lattice_size_list),
    temperature_list_(param.temperature_list),
    magnetic_h_list_(param.magnetic_h_list),
    iterations_(param.iterations),
    repetitions_(param.repetitions),
    size_list_size_(size_list_.size()),
    eval_cell_num_(temperature_list_.size() * magnetic_h_list_.size()),
    // Initialize `eval_list_` and `result_list_` with correct dimensions.
    eval_list_(size_list_size_, vector<LatticeDataUnit>(eval_cell_num_)),
    result_list_(size_list_size_,
        vector<vector<Lattice2D>>(eval_cell_num_, vector<Lattice2D>(repetitions_)))
{
    // Initialize `eval_list_` with correct `size` parameter.
    for (size_t i = 0; i != size_list_size_; ++i)
        for (size_t j = 0; j != eval_cell_num_; ++j)
            eval_list_[i][j] = LatticeDataUnit(repetitions_, size_list_[i]);
}

int LatticeData::Run()
{
    PrintParameters(cerr);
    Simulate();
    PrintResults(cout);

    return 0;
}

void LatticeData::Simulate()
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
        for (size_t j = 0; j < eval_cell_num_; ++j)
        {
            auto & eval = eval_list_[i][j];
            auto t = temperature_list_[j % kTemperatureListSize];
            auto h = magnetic_h_list_[j / kTemperatureListSize];
            eval.Run(t, h, iterations_);
            result_list_[i][j] = eval.Result();

            PrintProgress(eval_cell_num_, j + 1);
        }
        run_clock.TimingEnd();
        cerr << endl
             << "Running time: " << run_clock.GetRunningTime() << "s." << endl << endl;
    }

    cerr << "Finished!" << endl;
}

void LatticeData::PrintParameters(std::ostream & os)
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

void LatticeData::PrintResults(ostream & os)
{
    const auto kTemperatureListSize = temperature_list_.size();

    rapidjson::Document doc;
    doc.SetArray();
    auto & doc_allocator = doc.GetAllocator();

    for (size_t i = 0; i != size_list_size_; ++i)
        for (size_t j = 0; j != eval_cell_num_; ++j)
        {
            rapidjson::Value cell_val;
            cell_val.SetObject();

            // Parameters.
            cell_val.AddMember("size", size_list_[i], doc_allocator);
            cell_val.AddMember("temperature",
                temperature_list_[j % kTemperatureListSize], doc_allocator);
            cell_val.AddMember("externalMagneticField ",
                magnetic_h_list_[j / kTemperatureListSize], doc_allocator);

            // Lattice data.
            // 1st dimension:      repetition
            // 2nd, 3rd dimension: lattice
            rapidjson::Value lattice_data_val;
            rapidjson::Value lattice_val;
            rapidjson::Value row_val;
            lattice_data_val.SetArray();
            lattice_val.SetArray();
            row_val.SetArray();

            for (auto lattice : result_list_[i][j])
            {
                for (auto row : lattice)
                {
                    // `result` is a 2D lattice.
                    for (auto spin : row)
                        row_val.PushBack(spin, doc_allocator);
                    lattice_val.PushBack(row_val, doc_allocator);
                    row_val.Clear();
                }
                lattice_data_val.PushBack(lattice_val, doc_allocator);
                lattice_val.Clear();
            }
            
            cell_val.AddMember("latticeData", lattice_data_val, doc_allocator);

            // Add to the outer array.
            doc.PushBack(cell_val, doc_allocator);
        }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    string json_str(buffer.GetString());
    os << json_str << endl;
}

int RunLatticeData(const Parameter & param)
{
    LatticeData eval(param);
    return eval.Run();
}

ISING_NAMESPACE_END


/*
void PrintParameters(const Parameter & param)
{
    cerr << endl
         << "******************************" << endl
         << "Boundary condition: "
         << (param.boundary_condition == kPeriodic ? "Periodic" : "Free") << endl
         << "Lattice size:       "
         << param.lattice_size << endl
         << "Iterations:         "
         << param.iterations << endl
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
         << "******************************" << endl << endl;
}

typedef vector<LatticeInfo> ResultList;


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

        cell.Initialize();
        auto result = cell.EvaluateLatticeData(1.0 / temperature, h, param.iterations);
        result_list[i] = result;
    }
    run_clock.TimingEnd();
    cerr << endl
         << "Finished!" << endl
         << "Running time: " << run_clock.GetRunningTime() << "s." << endl;

    return result_list;
}


void PrintResults(ostream & os, const ResultList & result_list)
{
    Document doc;
    
    doc.SetArray();
    auto & doc_allocator = doc.GetAllocator();

    for (auto cell : result_list)
    {
        Value cell_val;
        cell_val.SetObject();

        Value energy_val;
        Value magnetic_dipole_val;
        energy_val.SetArray();
        magnetic_dipole_val.SetArray();

        for (auto i : cell.observables)
        {
            energy_val.PushBack(i.energy, doc_allocator);
            magnetic_dipole_val.PushBack(i.magnetic_dipole, doc_allocator);
        }

        Value lattice_data_val;
        lattice_data_val.SetArray();

        for (auto i : cell.lattice_data)
        {
            Value lattice_data_row_val;
            lattice_data_row_val.SetArray();
            for (auto j : i)
                lattice_data_row_val.PushBack(j, doc_allocator);
            lattice_data_val.PushBack(lattice_data_row_val, doc_allocator);
        }

        cell_val.AddMember("energy", energy_val, doc_allocator);
        cell_val.AddMember("magneticDipole", magnetic_dipole_val, doc_allocator);
        cell_val.AddMember("latticeData", lattice_data_val, doc_allocator);

        doc.PushBack(cell_val, doc_allocator);
    }
    
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);

    string json_str(buffer.GetString());
    os << json_str << endl;
}

void GenerateLatticeData(const Parameter & param)
{
    PrintParameters(param);

    size_t eval_list_size = param.temperature_list.size() * param.magnetic_h_list.size();
    ResultList result_list;

    if (param.boundary_condition == kPeriodic)
    {
        // Periodic boundary condition.
        vector<Ising2D_PBC> ising_eval_list(eval_list_size, param.lattice_size);
        result_list = Run(&ising_eval_list, param);
    }
    else
    {
        // Free boundary condition.
        vector<Ising2D_FBC> ising_eval_list(eval_list_size, param.lattice_size);
        result_list = Run(&ising_eval_list, param);
    }

    PrintResults(cout, result_list);
}
*/
