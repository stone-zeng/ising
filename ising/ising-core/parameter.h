#ifndef ISING_CORE_ISING_PARAMETER_H_
#define ISING_CORE_ISING_PARAMETER_H_

#include <string>
#include <vector>

#include <include/rapidjson/document.h>

#include "ising-core/ising.h"

ISING_NAMESPACE_BEGIN

// The settings file (JSON) may have the following keys:
//   "boundary"                    *  string ("periodic", "free")
//   "size.list"                      integer array
//   "temperature.list"               real-number array
//   "externalMagneticField.list"     real-number array
//   "size.span"                      object
//   "temperature.span"               object
//   "externalMagneticField.span"     object
//   "iterations"                  *  integer
//   "analysisEnsembleCount"       *  integer
//   "analysisEnsembleInterval"    *  integer
//   "repetitions"                 *  integer
//
// Keys with * have default values.
//
// A "span" object may have the following values:
//   "begin"    real-number / integer
//   "end"      real-number / integer
//   "step"     real-number / integer

class Parameter
{
public:
    Parameter() = default;

    void ReadFromString(const std::string & settings);
    void ReadFromString(const char * settings);
    void ReadFromFile(const std::string & file_name);
    void ReadFromFile(const char * file_name);

    void Parse();

    BoundaryCondition   boundary_condition;
    size_t              lattice_size;
    std::vector<size_t> lattice_size_list;
    std::vector<double> temperature_list;
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

    const double kDoubleTolerance = 1.0e-6;

    rapidjson::Document json_doc_;

    void ParseBoundaryCondition();
    void ParseLatticeSizeList();
    void ParseTemperatureList();
    void ParseMagneticFieldList();
    void ParseIterations();
    void ParseEnsembleCount();
    void ParseEnsembleInterval();
    void ParseRepetitions();
};

const std::string kDefaultSettingsString =
R"({
    "size.list": [4],
    "temperature.list": [1.0],
    "externalMagneticField.list": [0.0]
})";

// Allow relaxed JSON syntax (comments and trailing commas).
const auto kJsonParseFlag = rapidjson::kParseCommentsFlag
                          + rapidjson::kParseTrailingCommasFlag;

ISING_NAMESPACE_END

#endif
