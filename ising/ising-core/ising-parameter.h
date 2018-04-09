#ifndef ISING_CORE_ISING_PARAMETER_H_
#define ISING_CORE_ISING_PARAMETER_H_

#include <string>
#include <vector>

#include "include/rapidjson/document.h"

#include "ising-core/ising.h"
#include "ising-core/ising-definitions.h"

ISING_NAMESPACE_BEGIN

class Parameter
{
public:
    Parameter() = default;
    Parameter(const std::string & file_name);
    void Parse();

    BoundaryConditions  boundary_condition;
    LatticeSize         lattice_size;
    std::vector<double> beta_list;
    std::vector<double> magnetic_h_list;
    size_t              iterations;
    size_t              n_ensemble;
    size_t              n_delta;
    size_t              repetitions;

private:
    const size_t kDefaultIterations              = 1000;
    const size_t kDefaultIterationsEnsembleRatio = 10;
    const size_t kDefaultEnsembleInterval        = 1;
    const size_t kDefaultRepetitions             = 1;
    const double kDoubleTolerance                = 1.0e-6;

    rapidjson::Document json_doc_;

    BoundaryConditions  ParseBoundaryCondition();
    LatticeSize         ParseLatticeSize();
    std::vector<double> ParseBetaList();
    std::vector<double> ParseMagneticFieldList();
    size_t              ParseIterations();
    size_t              ParseEnsembleCount();
    size_t              ParseEnsembleInterval();
    size_t              ParseRepetitions();
};

ISING_NAMESPACE_END

#endif
