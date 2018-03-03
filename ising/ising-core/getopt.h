#ifndef GETOPT_H
#define GETOPT_H

#include <map>
#include <string>
#include <vector>

namespace Ising::Toolkit
{
    // Unix-like option "-a option_a -b option_b" parser.
    class GetOpt
    {
    public:
        GetOpt() = default;
        GetOpt(int argc, char * argv[]);

        std::string parse(char c) const;
        // int parse(std::string s);

    private:
        std::vector<std::string> raw_opt_list_;
        std::map<char, std::string> opt_key_val_list_;
    };
}

#endif
