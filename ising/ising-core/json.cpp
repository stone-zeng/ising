#include <iostream>
#include "json.h"

using namespace std;

JSON::JSON(const string & json_str)
{
    // The following code is just for parsing the structer like:
    // {
    //   "Size": 100,
    //   "Iterations": 1.2e6
    // }

    auto json_strsize = json_str.size();

    auto begin_pos = json_str.find('{');
    auto i = begin_pos, j = begin_pos;

    while (i < json_strsize)
    {
        i = json_str.find('"', i) + 1;
        j = json_str.find('"', i + 1);
        auto key = json_str.substr(i, j - i);

        i = json_str.find(':', j + 1);
        i = json_str.find_first_of("0123456789Ee.-", i + 1);
        j = json_str.find_first_of(" \n\r\t,}", i + 1);
        auto value = json_str.substr(i, j - i);

        key_value_list_.insert({ key, stod(value) });

        i = json_str.find_first_of(",", i + 1);
    }
}

double JSON::getNumberValue(const string & key)
{
    if (key_value_list_.find(key) == key_value_list_.end())
    {
        cerr << "Error";
        return NULL;
    }
    else
        return key_value_list_[key];
}
