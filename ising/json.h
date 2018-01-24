#ifndef JSON_H
#define JSON_H

#include <map>
#include <iostream>
#include <string>
#include <vector>

class JSON
{
public:
    JSON();
    JSON(const std::string & json_str);
    ~JSON();

    std::string raw_str;

    double getNumberValue(const std::string & key);

private:
    std::map<std::string, double> key_value_list_;
};

JSON::JSON()
{}

JSON::JSON(const std::string & json_str)
{
    // The following code is just for parsing the structer like:
    // {
    //   "Size": 100,
    //   "Iterations": 1.2e6
    // }

    raw_str = json_str;
    auto raw_str_size = raw_str.size();

    auto begin_pos = raw_str.find('{');
    auto i = begin_pos, j = begin_pos;

    while (i < raw_str_size)
    {
        i = raw_str.find('"', i) + 1;
        j = raw_str.find('"', i + 1);
        auto key = raw_str.substr(i, j - i);

        i = raw_str.find(':', j + 1);
        i = raw_str.find_first_of("0123456789Ee.-", i + 1);
        j = raw_str.find_first_of(" \n\r\t,}", i + 1);
        auto value = raw_str.substr(i, j - i);

        key_value_list_.insert({ key, std::stod(value) });

        i = raw_str.find_first_of(",", i + 1);
    }
}

JSON::~JSON()
{}

double JSON::getNumberValue(const std::string & key)
{
    if (key_value_list_.find(key) == key_value_list_.end())
    {
        std::cerr << "Error";
        return NULL;
    }
    else
        return key_value_list_[key];
}

#endif
