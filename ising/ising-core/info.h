#ifndef ISING_CORE_INFO_H_
#define ISING_CORE_INFO_H_

#include <string>

#include "ising-core/ising.h"

ISING_TOOLKIT_NAMESPACE_BEGIN

std::string InformationSeparator();
void PrintProgress(const size_t & total, const size_t & progress);

ISING_TOOLKIT_NAMESPACE_END

#endif
