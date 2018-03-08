#ifndef ISING_CORE_ISING_PARAMETER_H_
#define ISING_CORE_ISING_PARAMETER_H_

#include <string>
#include <vector>

#include "include/rapidjson/document.h"

#include "ising.h"
#include "ising-definitions.h"

ISING_NAMESPACE_BEGIN

class Parameter
{
public:
    Parameter() = default;
    Parameter(const std::string & file_name);

    typedef struct { size_t x; size_t y; } Size;

    BoundaryConditions boundary_condition_;
    LatticeSize size_;
    std::vector<double> beta_list_;
    std::vector<double> magnetic_h_list_;
    size_t iterations_;
    size_t n_ensemble_;
    size_t n_delta_;

private:
    const size_t kDefaultIterations              = 1000;
    const size_t kDefaultIterationsEnsembleRatio = 10;
    const size_t kDefaultEnsembleInterval        = 1;
    const double kDoubleTolerance                = 1.0e-6;

    std::string raw_json_str_;

    void Parse();
    BoundaryConditions ParseBoundaryCondition(const rapidjson::Document & doc);
    LatticeSize ParseSize(const rapidjson::Document & doc);
    std::vector<double> ParseBetaList(const rapidjson::Document & doc);
    std::vector<double> ParseMagneticFieldList(const rapidjson::Document & doc);
    size_t ParseIterations(const rapidjson::Document & doc);
    size_t ParseEnsembleCount(const rapidjson::Document & doc);
    size_t ParseEnsembleInterval(const rapidjson::Document & doc);
};

ISING_NAMESPACE_END

#endif
