#ifndef JSON_H
#define JSON_H

#include <map>
#include <string>

namespace Ising::Toolkit
{
    class JSON
    {
    public:
        JSON() = default;
        JSON(const std::string & json_str);

        double getNumberValue(const std::string & key) const;

    private:
        std::map<std::string, double> key_value_list_;
    };
}

#endif
