#ifndef ISING_OPTION_H
#define ISING_OPTION_H

#include <string>
#include <vector>

#include "include/rapidjson/document.h"
#include "ising-definitions.h"

namespace Ising
{
    class Option
    {
    public:
        Option() = default;
        Option(const std::string & file_name);

        BoundaryTypes boundary_type_;
        std::vector<double> beta_list_;

    private:
        typedef rapidjson::Document JSON;

        std::string raw_json_str_;

        void parse();
        BoundaryTypes parseBoundaryType(const JSON & json);
        std::vector<double> parseBetaList(const JSON & json);
    };
}

#endif
