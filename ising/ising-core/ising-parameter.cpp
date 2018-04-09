#include "ising-core/ising-parameter.h"

#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "include/rapidjson/document.h"

#include "ising-core/ising.h"
#include "ising-core/ising-definitions.h"

using namespace std;
using namespace rapidjson;

ISING_NAMESPACE_BEGIN

bool _LessEqual(const double & a, const double & b, const double & tolerance)
{
    return a - b <= fabs(tolerance);
}

vector<double> _SpanToVector(const Value & span, const double & tolerance)
{
    vector<double> v;
    auto v_begin = span["begin"].GetDouble();
    auto v_end   = span["end"].GetDouble();
    auto v_step  = span["step"].GetDouble();
    for (double i = v_begin; _LessEqual(i, v_end, tolerance); i += v_step)
        v.push_back(i);
    return v;
}

size_t _GetSizeType(const Document & doc, const char key[], const size_t & default_value)
{
    auto iter = doc.FindMember(key);
    if (iter != doc.MemberEnd())
        return static_cast<size_t>(iter->value.GetInt());
    else
        return default_value;
}

vector<double> _GetVector(const Value & array)
{
    vector<double> v;
    for (auto & i : array.GetArray())
        v.push_back(i.GetDouble());
    return v;
}

Parameter::Parameter(const string & file_name)
{
    ifstream file(file_name);
    // Read the whole file. See https://stackoverflow.com/a/116220/8479490.
    string json_string = static_cast<stringstream const&>(stringstream() << file.rdbuf()).str();
    // Allow relaxed JSON syntax (comments and trailing commas).
    json_doc_.Parse<kParseCommentsFlag + kParseTrailingCommasFlag>(json_string.c_str());
    file.close();
}

void Parameter::Parse()
{
    boundary_condition = ParseBoundaryCondition();
    lattice_size       = ParseLatticeSize();
    beta_list          = ParseBetaList();
    magnetic_h_list    = ParseMagneticFieldList();
    iterations         = ParseIterations();
    n_ensemble         = ParseEnsembleCount();
    n_delta            = ParseEnsembleInterval();
    repetitions        = ParseRepetitions();
}

BoundaryConditions Parameter::ParseBoundaryCondition()
{
    if (json_doc_.HasMember("boundary") && strcmp(json_doc_["boundary"].GetString(), "free") == 0)
        return kFree;
    else
        return kPeriodic;
}

LatticeSize Parameter::ParseLatticeSize()
{
    auto iter = json_doc_.FindMember("size");
    if (iter != json_doc_.MemberEnd())
    {
        size_t size = static_cast<size_t>(iter->value.GetInt());
        return LatticeSize{ size, size };
    }
    else
    {
        size_t x_size = static_cast<size_t>(json_doc_["xSize"].GetInt());
        size_t y_size = static_cast<size_t>(json_doc_["ySize"].GetInt());
        return LatticeSize{ x_size, y_size };
    }
}

vector<double> Parameter::ParseBetaList()
{
    // Try to find `temperature` first.
    auto span_iter = json_doc_.FindMember("temperature.span");
    auto list_iter = json_doc_.FindMember("temperature.list");
    if (span_iter != json_doc_.MemberEnd() || list_iter != json_doc_.MemberEnd())
    {
        vector<double> list;
        if (span_iter != json_doc_.MemberEnd())
            list = _SpanToVector(span_iter->value, kDoubleTolerance);
        else
            list = _GetVector(list_iter->value);
        for (auto & i : list)
            i = 1.0 / i;
        return list;
    }
    else
    {
        span_iter = json_doc_.FindMember("beta.span");
        if (span_iter != json_doc_.MemberEnd())
            return _SpanToVector(span_iter->value, kDoubleTolerance);
        list_iter = json_doc_.FindMember("beta.list");
        if (list_iter != json_doc_.MemberEnd())
            return _GetVector(list_iter->value);
    }
    // The program should not go here.
    return vector<double>();
}

vector<double> Parameter::ParseMagneticFieldList()
{
    auto span_iter = json_doc_.FindMember("externalMagneticField.span");
    if (span_iter != json_doc_.MemberEnd())
        return _SpanToVector(span_iter->value, kDoubleTolerance);
    auto list_iter= json_doc_.FindMember("externalMagneticField.list");
    if (list_iter != json_doc_.MemberEnd())
        return _GetVector(list_iter->value);
    // The program should not go here.
    return vector<double>();
}

size_t Parameter::ParseIterations()
{
    return _GetSizeType(json_doc_, "iterations", kDefaultIterations);
}

size_t Parameter::ParseEnsembleCount()
{
    return _GetSizeType(json_doc_, "analysisEnsembleCount",
        iterations / kDefaultIterationsEnsembleRatio);
}

size_t Parameter::ParseEnsembleInterval()
{
    return _GetSizeType(json_doc_, "analysisEnsembleInterval", kDefaultEnsembleInterval);
}

size_t Parameter::ParseRepetitions()
{
    return _GetSizeType(json_doc_, "repetitions", kDefaultRepetitions);
}

ISING_NAMESPACE_END
