#include <iostream>
#include <string>
#include <vector>

#include "include/rapidjson/document.h"
#include "include/rapidjson/writer.h"

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
using namespace rapidjson;
using namespace ising;
using namespace ising::toolkit;

void PrintParameters(const Parameter & param)
{
    cerr << endl
         << "******************************" << endl
         << "Boundary condition: "
         << (param.boundary_condition == kPeriodic ? "Periodic" : "Free") << endl
         << "Lattice size:       "
         << param.lattice_size.x << "*" << param.lattice_size.y << endl
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
//ResultList Run(vector<Ising2D> * eval_list, const Parameter & param)
{
    const auto & beta_list = param.beta_list;
    const auto & h_list    = param.magnetic_h_list;
    size_t beta_list_size  = beta_list.size();
    size_t h_list_size     = h_list.size();
    size_t list_size       = beta_list_size * h_list_size;

    ResultList result_list;
    result_list.resize(list_size);

    Timing run_clock;

    cerr << "Running..." << endl;
    run_clock.TimingStart();
#ifdef ISING_PARALLEL
#pragma omp parallel for
#endif
    for (auto i = 0; i < list_size; ++i)
    {
        auto beta = beta_list[i % beta_list_size];
        auto h = h_list[i / beta_list_size];
        auto & cell = (*eval_list)[i];

        cell.Initialize();
        auto result = cell.EvaluateLatticeData(beta, h, param.iterations);
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

    size_t eval_list_size = param.beta_list.size() * param.magnetic_h_list.size();
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

void GenerateDumpedLatticeData(const Parameter & param)
{
}

int main(int argc, char * argv[])
{
    GetOption option(argc, argv);
    Parameter parameter(option.Parse('s'));
    parameter.Parse();

    if (option.Parse('d') == "true")
        GenerateDumpedLatticeData(parameter);
    else
        GenerateLatticeData(parameter);

    return 0;
}
