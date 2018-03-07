#ifndef ISING_CORE_GETOPT_H_
#define ISING_CORE_GETOPT_H_

#include <map>
#include <string>
#include <vector>

#include "ising.h"

ISING_TOOLKIT_NAMESPACE_BEGIN

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

ISING_TOOLKIT_NAMESPACE_END

#endif
