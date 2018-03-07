#ifndef ISING_CORE_ISING_OPTION_H_
#define ISING_CORE_ISING_OPTION_H_

#include <string>
#include <vector>

#include "include/rapidjson/document.h"

#include "ising.h"
#include "ising-definitions.h"

ISING_NAMESPACE_BEGIN

class Option
{
public:
    Option() = default;
    Option(const std::string & file_name);

    BoundaryTypes boundary_type_;
    std::vector<double> beta_list_;

private:
    std::string raw_json_str_;

    void Parse();
    BoundaryTypes ParseBoundaryType(const rapidjson::Document & json);
    std::vector<double> ParseBetaList(const rapidjson::Document & json);
};

ISING_NAMESPACE_END

#endif
