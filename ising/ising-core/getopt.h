#ifndef GETOPT_H
#define GETOPT_H

#include <map>
#include <string>
#include <vector>

// Unix-like option "-a option_a -b option_b" parser.
class GetOpt
{
public:
    GetOpt();
    GetOpt(int argc, char * argv[]);
    ~GetOpt();
    std::string parse(char c);
    // int parse(std::string s);

private:
    std::vector<std::string> raw_opt_list_;
    std::map<char, std::string> opt_key_val_list_;

    inline bool isOption(std::string s)
    {
        if (s.empty())
            return false;
        else
            return s[0] == '-';
    }
};

GetOpt::GetOpt()
{}

GetOpt::GetOpt(int argc, char * argv[])
{
    for (auto i = 0; i != argc; ++i)
        raw_opt_list_.push_back(argv[i]);

    for (auto iter = raw_opt_list_.begin(); iter != raw_opt_list_.end(); )
    {
        if (isOption(*iter))
        {
            auto key = (*iter)[1];
            ++iter;
            std::string val;
            if (isOption(*iter))
                val = "";
            else
                val = *iter;

            opt_key_val_list_.insert({ key,val });
        }
        else
            ++iter;
    }
}

GetOpt::~GetOpt()
{}

std::string GetOpt::parse(char c)
{
    return opt_key_val_list_[c];
}

#endif
