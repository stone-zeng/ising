#include "stdafx.h"

#include "../ising-core/fast-rand.h"
#include "../ising-core/getopt.h"
#include "../ising-core/json.h"
#include "../ising-core/win-timing.h"

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Ising::Toolkit;

namespace Ising::Test
{
    TEST_CLASS(AuxiliariesTest)
    {
    public:
        TEST_METHOD(FastRandTest)
        {
            const size_t test_num = 100;
            const size_t show_num = 5;
            auto rand_seed = static_cast<unsigned int>(time(NULL));

            Timing clock;
            string message;

            vector<int> fast_rand_result(test_num);
            vector<int> std_rand_result  (test_num);
            vector<int> cpp11_rand_result(test_num);

#define _RAND_TEST_RESULT(_f, _result)  \
    clock.timingStart();                \
    for (auto & i : (_result))          \
        i = (_f);                       \
    clock.timingEnd();

#define _RAND_TEST_MESSAGE(_head, _result)                  \
    message = (_head);                                      \
    message += to_string(clock.getRunTime("ms")) + "ms. ";  \
    message += "Result: ";                                  \
    for (auto i = 0; i != show_num;                         \
            ++i)                                            \
        message += to_string((_result)[i]) + " ";           \
    message += "...";                                       \
    Logger::WriteMessage(message.c_str());

#define _RAND_TEST(_f, _result, _head)      \
    _RAND_TEST_RESULT((_f), (_result))      \
    _RAND_TEST_MESSAGE((_head), (_result))

            // My fast random integer.
            fastRandInitialize(rand_seed);
            _RAND_TEST(fastRand(), fast_rand_result, "fastRand(): ")

            // C standard random integer.
            srand(rand_seed);
            _RAND_TEST(rand(), std_rand_result, "C rand(): ")

            // C++ 11 STL random library.
            static std::default_random_engine engine;
            static std::uniform_int_distribution<> dist(0, RAND_MAX);
            engine.seed(rand_seed);
            _RAND_TEST(dist(engine), cpp11_rand_result, "C++ 11 random: ")

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
