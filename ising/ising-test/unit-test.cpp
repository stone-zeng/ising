#include "stdafx.h"
#include "CppUnitTest.h"

#include <cstdlib>
#include <vector>
#include <string>
#include "../fast-rand.h"
#include "../getopt.h"
#include "../json.h"

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace IsingTest
{
    TEST_CLASS(AuxiliariesTest)
    {
    public:
        TEST_METHOD(FastRandTest)
        {
            const size_t test_num = 10;

            // Set `rand_seed` to be an arbitrary number
            const unsigned int rand_seed = 1602;
            g_seed = rand_seed;
            srand(rand_seed);

            vector<int> fast_rand_result;
            vector<int> std_rand_result;

            for (int i = 0; i != test_num; ++i)
            {
                fast_rand_result.push_back(fastRand());
                std_rand_result.push_back(rand());
            }

            Assert::IsTrue(std_rand_result == fast_rand_result);
        }

        TEST_METHOD(GetOptTest)
        {
            char * arg_exe_name = "test.exe";
            char * arg_n = "123";
            char * arg_o = "options";

            char * argv[] = { arg_exe_name, "-n", arg_n, "-o", arg_o };
            int argc = static_cast<int>(std::size(argv));

            GetOpt options(argc, argv);

            Assert::AreEqual(std::atoi(arg_n), std::stoi(options.parse('n')));
            Assert::AreEqual(std::string(arg_o), options.parse('o'));
        }

        TEST_METHOD(JSONTest)
        {
            const string json_begin = "{";
            const string json_end = "}";
            const string json_keyval_seperator = ": ";
            const string json_item_seperator = ",\n";

            string key_1 = "Size";
            string key_2 = "Iterations";
            double value_1 = 100;
            double value_2 = 1e6;

            string keyval_1 = "\"" + key_1 + "\"" + json_keyval_seperator + to_string(value_1);
            string keyval_2 = "\"" + key_2 + "\"" + json_keyval_seperator + to_string(value_2);

            string json_str = json_begin + "\n"
                + "\t" + keyval_1 + json_item_seperator
                + "\t" + keyval_2 + "\n"
                + json_end;

            JSON json(json_str);

            Assert::AreEqual(value_1, json.getNumberValue(key_1));
            Assert::AreEqual(value_2, json.getNumberValue(key_2));
        }
    };
}
