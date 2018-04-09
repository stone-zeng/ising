#ifndef ISING_CORE_GET_OPTION_H_
#define ISING_CORE_GET_OPTION_H_

#include <map>
#include <string>
#include <vector>

#include "ising-core/ising.h"

ISING_TOOLKIT_NAMESPACE_BEGIN

// Unix-like option "-a option_a -b option_b" parser.
class GetOption
{
public:
    GetOption() = default;
    GetOption(int argc, char * argv[]);

    std::string Parse(const char & c) const;
    // int Parse(std::string s);

private:
    std::map<char, std::string> option_;
};

ISING_TOOLKIT_NAMESPACE_END

#endif
