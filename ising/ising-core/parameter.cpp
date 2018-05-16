#include "ising-core/parameter.h"

#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <typeinfo>
#include <vector>
#include <include/rapidjson/document.h>

#include "ising-core/ising.h"

using namespace std;

ISING_NAMESPACE_BEGIN

void Parameter::ReadFromString(const string & settings) { ReadFromString(settings.c_str()); }
void Parameter::ReadFromString(const char * settings)
{
    json_doc_.Parse<kJsonParseFlag>(settings);
}

void Parameter::ReadFromFile(const string & file_name) { ReadFromFile(file_name.c_str()); }
void Parameter::ReadFromFile(const char * file_name)
{
    ifstream file(file_name);
    // Read the whole file. See https://stackoverflow.com/a/116220/8479490.
    string json_string = static_cast<stringstream const&>(stringstream() << file.rdbuf()).str();
    json_doc_.Parse<kJsonParseFlag>(json_string.c_str());
    file.close();
}

void Parameter::Parse()
{
    ParseBoundaryCondition();
    ParseLatticeSizeList();
    ParseTemperatureList();
    ParseMagneticFieldList();
    ParseIterations();
    ParseEnsembleCount();
    ParseEnsembleInterval();
    ParseRepetitions();
}

void Parameter::ParseBoundaryCondition()
{
    if (json_doc_.HasMember("boundary") && strcmp(json_doc_["boundary"].GetString(), "free") == 0)
        boundary_condition = kFree;
    else
        boundary_condition = kPeriodic;
}

template <typename T>
bool _LessEqual(const T & a, const T & b, const double & tolerance)
{
    if(typeid(T) == typeid(double))
        return a - b <= fabs(tolerance);
    else
        return a <= b;
}

template <typename T>
vector<T> _SpanToVector(const rapidjson::Value & span, const double & tolerance)
{
    vector<T> v;
    auto v_begin = span["begin"].Get<T>();
    auto v_end   = span["end"].Get<T>();
    auto v_step  = span["step"].Get<T>();
    for (auto i = v_begin; _LessEqual(i, v_end, tolerance); i += v_step)
        v.push_back(i);
    return v;
}

template <typename T>
vector<T> _GetVector(const rapidjson::Value & array)
{
    vector<T> v;
    for (auto & i : array.GetArray())
        v.push_back(i.Get<T>());
    return v;
}

// Helper function for getting a list.
// The value can be specified via either a "span" or a "list".
// If both "span" and "list" are given, "span" will be used.
// `tolerance` will be ignored except for `double`.
template <typename T>
vector<T> _ParseList(const rapidjson::Document & doc, const string & key, const double & tolerance)
{
    auto span_iter = doc.FindMember((key + ".span").c_str());
    if (span_iter != doc.MemberEnd())
        return _SpanToVector<T>(span_iter->value, tolerance);
    auto list_iter = doc.FindMember((key + ".list").c_str());
    if (list_iter != doc.MemberEnd())
        return _GetVector<T>(list_iter->value);
    // The program should never go here.
    return vector<T>();
}

template <typename T>
vector<T> _ParseList(const rapidjson::Document & doc, const string & key)
{
    return _ParseList<T>(doc, key, 0.0);
}

void Parameter::ParseLatticeSizeList()
{
    lattice_size_list = _ParseList<size_t>(json_doc_, "size");
    if (lattice_size_list.empty() == false)
        lattice_size = lattice_size_list[0];
}

void Parameter::ParseTemperatureList()
{
    temperature_list = _ParseList<double>(json_doc_, "temperature", kDoubleTolerance);
}

void Parameter::ParseMagneticFieldList()
{
    magnetic_h_list = _ParseList<double>(json_doc_, "externalMagneticField", kDoubleTolerance);
}

// Helper function for getting a `size_t` value.
size_t _ParseSizeT(const rapidjson::Document & doc, const char * key, const size_t & default_value)
{
    auto iter = doc.FindMember(key);
    if (iter != doc.MemberEnd())
        return static_cast<size_t>(iter->value.GetInt());
    else
        return default_value;
}

void Parameter::ParseIterations()
{
    iterations = _ParseSizeT(json_doc_, "iterations", kDefaultIterations);
}

void Parameter::ParseEnsembleCount()
{
    n_ensemble = _ParseSizeT(json_doc_, "analysisEnsembleCount",
        iterations / kDefaultIterationsEnsembleRatio);
}

void Parameter::ParseEnsembleInterval()
{
    n_delta = _ParseSizeT(json_doc_, "analysisEnsembleInterval", kDefaultEnsembleInterval);
}

void Parameter::ParseRepetitions()
{
    repetitions = _ParseSizeT(json_doc_, "repetitions", kDefaultRepetitions);
}

ISING_NAMESPACE_END
