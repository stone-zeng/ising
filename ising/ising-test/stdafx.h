// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef ISING_TEST_STDAFX_H_
#define ISING_TEST_STDAFX_H_

#include "targetver.h"

// Headers for CppUnitTest
#include <CppUnitTest.h>

// TODO: reference additional headers your program requires here
#include <cstdlib>
#include <ctime>
#include <random>
#include <string>
#include <vector>

#include <Windows.h>

#include <include/rapidjson/document.h>
#include <include/argagg/argagg.hpp>

#define PRINT_TEST_INFO(_name)                  \
    std::string info_head(">> Begin test: ");   \
    info_head += (_name);                       \
    Microsoft::VisualStudio::CppUnitTestFramework::Logger::WriteMessage(info_head.c_str());

#define ISING_TEST_NAMESPACE_BEGIN namespace ising { namespace test {
#define ISING_TEST_NAMESPACE_END } }

#endif
