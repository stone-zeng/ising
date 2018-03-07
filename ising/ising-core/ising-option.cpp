#include "ising-option.h"

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

Option::Option(const string & file_name)
{
    ifstream file(file_name);
    file >> raw_json_str_;
    Parse();
}

void Option::Parse()
{
    JSON json;
    json.Parse(raw_json_str_.c_str());

    boundary_type_ = parseBoundaryType(json);
    beta_list_     = parseBetaList(json);
}

BoundaryTypes Option::parseBoundaryType(const JSON & json)
{
    auto s = json["boundary"].GetString();
    if (strcmp(s, "periodic"))
        return kPeriodic;
    if (strcmp(s, "free"))
        return kFree;
}

vector<double> Option::parseBetaList(const JSON & json)
{
    vector<double> beta_list;

    if (json.HasMember("beta"))
    {
        auto beta_list_begin = json["beta"]["begin"].GetDouble();
        auto beta_list_end   = json["beta"]["end"].GetDouble();
        auto beta_list_step  = json["beta"]["step"].GetDouble();
        for (auto i = beta_list_begin; i < beta_list_end; i += beta_list_step)
            beta_list.push_back(i);
    }
    else
    {
        auto temp_list_begin = json["temperature"]["begin"].GetDouble();
        auto temp_list_end   = json["temperature"]["end"].GetDouble();
        auto temp_list_step  = json["temperature"]["step"].GetDouble();
        for (auto i = temp_list_begin; i < temp_list_end; i += temp_list_step)
            beta_list.push_back(1 / i);
    }

    return beta_list;
}

ISING_NAMESPACE_END
