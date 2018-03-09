#include "get-option.h"

#include <map>
#include <string>
#include <vector>

#include "ising.h"

using namespace std;

ISING_TOOLKIT_NAMESPACE_BEGIN

bool _IsOption(const string & s)
{
    if (s.empty())
        return false;
    else
        return s[0] == '-';
}

GetOption::GetOption(int argc, char * argv[])
{
    std::vector<std::string> raw_option;
    for (auto i = 0; i != argc; ++i)
        raw_option.push_back(argv[i]);
    for (auto iter = raw_option.begin(); iter != raw_option.end(); )
    {
        if (_IsOption(*iter))
        {
            auto key = (*iter)[1];
            ++iter;
            string val;
            if (_IsOption(*iter))
                val = "";
            else
                val = *iter;
            option_.insert({ key,val });
        }
        else { ++iter; }
    }
}

string GetOption::Parse(const char & c) const { return option_.at(c); }

ISING_TOOLKIT_NAMESPACE_END
