// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef STDAFX_H
#define STDAFX_H

#include "targetver.h"

// Headers for CppUnitTest
#include <CppUnitTest.h>

// TODO: reference additional headers your program requires here
#include <cstdlib>
#include <ctime>
#include <random>
#include <string>
#include <vector>

#include "ising-core/include/rapidjson/document.h"

#define PRINT_TEST_INFO(_name)                  \
    std::string info_head(">> Start test: ");   \
    info_head += (_name);                       \
    Microsoft::VisualStudio::CppUnitTestFramework::Logger::WriteMessage(info_head.c_str());

#endif
