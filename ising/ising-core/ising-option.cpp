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
    // Read the whole file. See https://stackoverflow.com/a/116220/8479490.
    raw_json_str_ = static_cast<stringstream const&>(stringstream() << file.rdbuf()).str();
    Parse();
}

void Option::Parse()
{
    Document doc;
    // Allow relaxed JSON syntax (comments and trailing commas).
    doc.Parse<kParseCommentsFlag + kParseTrailingCommasFlag>(raw_json_str_.c_str());

    boundary_type_ = ParseBoundaryType(doc);
    beta_list_     = ParseBetaList(doc);
}

BoundaryTypes Option::ParseBoundaryType(const Document & json)
{
    string s(json["boundary"].GetString());
    if (s == "periodic")
        return kPeriodic;
    if (s == "free")
        return kFree;
}

vector<double> Option::ParseBetaList(const Document & json)
{
    vector<double> beta_list;

    if (json.HasMember("beta"))
    {
        auto beta_list_begin = json["beta"]["begin"].GetDouble();
        auto beta_list_end   = json["beta"]["end"].GetDouble();
        auto beta_list_step  = json["beta"]["step"].GetDouble();
        for (auto i = beta_list_begin; i <= beta_list_end + beta_list_step; i += beta_list_step)
            beta_list.push_back(i);
    }
    else
    {
        auto temp_list_begin = json["temperature"]["begin"].GetDouble();
        auto temp_list_end   = json["temperature"]["end"].GetDouble();
        auto temp_list_step  = json["temperature"]["step"].GetDouble();
        for (auto i = temp_list_begin; i <= temp_list_end + temp_list_step; i += temp_list_step)
            beta_list.push_back(1 / i);
    }

    return beta_list;
}

ISING_NAMESPACE_END
