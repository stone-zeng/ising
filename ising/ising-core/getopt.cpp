#include "getopt.h"

using namespace std;

namespace Ising::Toolkit
{
    bool _isOption(string s)
    {
        if (s.empty())
            return false;
        else
            return s[0] == '-';
    }

    GetOpt::GetOpt(int argc, char * argv[])
    {
        for (auto i = 0; i != argc; ++i)
            raw_opt_list_.push_back(argv[i]);

        for (auto iter = raw_opt_list_.begin(); iter != raw_opt_list_.end(); )
        {
            if (_isOption(*iter))
            {
                auto key = (*iter)[1];
                ++iter;
                string val;
                if (_isOption(*iter))
                    val = "";
                else
                    val = *iter;

                opt_key_val_list_.insert({ key,val });
            }
            else
                ++iter;
        }
    }

    string GetOpt::parse(char c) const { return opt_key_val_list_.at(c); }
}
