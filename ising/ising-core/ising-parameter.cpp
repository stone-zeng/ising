#include "ising-parameter.h"

#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "include/rapidjson/document.h"

#include "ising.h"
#include "ising-definitions.h"

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
    raw_json_str_ = static_cast<stringstream const&>(stringstream() << file.rdbuf()).str();
    Parse();
}

void Parameter::Parse()
{
    Document doc;
    // Allow relaxed JSON syntax (comments and trailing commas).
    doc.Parse<kParseCommentsFlag + kParseTrailingCommasFlag>(raw_json_str_.c_str());
    // Get all variables.
    boundary_condition_ = ParseBoundaryCondition(doc);
    size_               = ParseSize(doc);
    beta_list_          = ParseBetaList(doc);
    magnetic_h_list_    = ParseMagneticFieldList(doc);
    iterations_         = ParseIterations(doc);
    n_ensemble_         = ParseEnsembleCount(doc);
    n_delta_            = ParseEnsembleInterval(doc);
}

BoundaryConditions Parameter::ParseBoundaryCondition(const Document & doc)
{
    if (doc.HasMember("boundary") && strcmp(doc["boundary"].GetString(), "free") == 0)
        return kFree;
    else
        return kPeriodic;
}

LatticeSize Parameter::ParseSize(const Document & doc)
{
    auto iter = doc.FindMember("size");
    if (iter != doc.MemberEnd())
    {
        size_t size = static_cast<size_t>(iter->value.GetInt());
        return LatticeSize{ size, size };
    }
    else
    {
        size_t x_size = static_cast<size_t>(doc["xSize"].GetInt());
        size_t y_size = static_cast<size_t>(doc["ySize"].GetInt());
        return LatticeSize{ x_size, y_size };
    }
}

vector<double> Parameter::ParseBetaList(const Document & doc)
{
    // `temp` = temperature
    vector<double> beta_list;

    auto span_iter = doc.FindMember("temperature.span");
    auto list_iter = doc.FindMember("temperature.list");

    if (span_iter != doc.MemberEnd() || list_iter != doc.MemberEnd())
    {
        if (span_iter != doc.MemberEnd())
            beta_list = _SpanToVector(span_iter->value, kDoubleTolerance);
        else
            beta_list = _GetVector(list_iter->value);
        for (auto & i : beta_list)
            i = 1.0 / i;
        return beta_list;
    }
    else
    {
        span_iter = doc.FindMember("beta.span");
        if (span_iter != doc.MemberEnd())
            return _SpanToVector(span_iter->value, kDoubleTolerance);
        list_iter = doc.FindMember("beta.list");
        if (list_iter != doc.MemberEnd())
            return _GetVector(list_iter->value);
        // ELSE: Error
    }
}

vector<double> Parameter::ParseMagneticFieldList(const Document & doc)
{
    auto span_iter = doc.FindMember("externalMagneticField.span");
    if (span_iter != doc.MemberEnd())
        return _SpanToVector(span_iter->value, kDoubleTolerance);
    auto list_iter= doc.FindMember("externalMagneticField.list");
    if (list_iter != doc.MemberEnd())
        return _GetVector(list_iter->value);
    // ELSE: Error
}

size_t Parameter::ParseIterations(const Document & doc)
{
    return _GetSizeType(doc, "iterations", kDefaultIterations);
}

size_t Parameter::ParseEnsembleCount(const Document & doc)
{
    return _GetSizeType(doc, "analysisEnsembleCount", iterations_ / kDefaultIterationsEnsembleRatio);
}

size_t Parameter::ParseEnsembleInterval(const Document & doc)
{
    return _GetSizeType(doc, "analysisEnsembleInterval", kDefaultEnsembleInterval);
}

ISING_NAMESPACE_END
