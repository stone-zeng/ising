#ifndef ISING_CORE_SIMULATION_H_
#define ISING_CORE_SIMULATION_H_

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <include/argagg/argagg.hpp>

#include "ising-core/ising.h"
#include "ising-core/ising-2d.h"
#include "ising-core/parameter.h"

// "Windows.h" should be put after "rapidjson/document.h".
// See https://github.com/Tencent/rapidjson/issues/766
//     http://blog.csdn.net/u011519892/article/details/16985239
#include "ising-core/timing.h"

//using namespace std;
//using namespace ising;
//using namespace ising::toolkit;

ISING_NAMESPACE_BEGIN

// Each cell will be initialized with the same parameters (size, T, H).
// Repeat the evaluation for `repetitions` times for calculating deviation etc.
class EvalCell
{
public:
    EvalCell() = default;
    EvalCell(const size_t & repetitions, const size_t & lattice_size);
    
    void Run(const double & temperature, const double & magnetic_h,
        const size_t & iterations, const size_t & n_ensemble, const size_t & n_delta);
    inline std::vector<Observable> Result() { return result_list_; }

private:
    std::vector<Ising2D_PBC> eval_list_;
    std::vector<Observable>  result_list_;
};

// 
class Simulation
{
public:
    Simulation() = default;
    Simulation(const Parameter & param);

    int Run();

private:
    // Parameters and parameter lists.
    const std::vector<size_t> size_list_;
    const std::vector<double> temperature_list_;
    const std::vector<double> magnetic_h_list_;
    const size_t              iterations_;
    const size_t              n_ensemble_;
    const size_t              n_delta_;
    const size_t              repetitions_;

    // The dimension of T * B
    const size_t eval_cell_num_;

    // // Inner dimension is used for repetition.
    // //typedef std::vector<Ising2D> EvalCell;
    // typedef std::vector<EvaluationResult> ResultList;

    // 1st dimension: size
    // 2nd dimension: T * B
    // 3rd dimension (in `EvalCell`): repetition
    std::vector<std::vector<EvalCell>> eval_list_;

    // 1st dimension: size * T * B
    // 2nd dimension: repetition
    std::vector<std::vector<Observable>> result_list_;
    
    void Simulate();
    void PrintParameters(std::ostream & os);
    void PrintResults(std::ostream & os);
    void PrintProgress(const size_t & total, const size_t & progress);
};

// Interface.
int RunSimulation(const Parameter & param);

ISING_NAMESPACE_END

#endif
