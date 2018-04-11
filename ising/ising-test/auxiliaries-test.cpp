#include "stdafx.h"

#include "ising-core/fast-rand.h"
#include "ising-core/win-timing.h"

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace ising::toolkit;

ISING_TEST_NAMESPACE_BEGIN

TEST_CLASS(AuxiliariesTest)
{
public:
    TEST_METHOD(FastRandTest)
    {
        PRINT_TEST_INFO("Fast rand")

        size_t test_num = 100;
        size_t show_num = 5;
        auto rand_seed = static_cast<unsigned int>(time(NULL));

        Timing clock;
        string message;

        vector<int> fast_rand_result (test_num);
        vector<int> std_rand_result  (test_num);
        vector<int> cpp11_rand_result(test_num);

#define _RAND_TEST_RESULT(_f, _result)  \
    clock.TimingStart();                \
    for (auto & i : (_result))          \
        i = (_f);                       \
    clock.TimingEnd();

#define _RAND_TEST_MESSAGE(_head, _result)                      \
    message = (_head);                                          \
    message += to_string(clock.GetRunningTime("ms")) + "ms. ";  \
    message += "Result: ";                                      \
    for (auto i = 0; i != show_num;                             \
            ++i)                                                \
        message += to_string((_result)[i]) + " ";               \
    message += "...";                                           \
    Logger::WriteMessage(message.c_str());

#define _RAND_TEST(_f, _result, _head)      \
    _RAND_TEST_RESULT((_f), (_result))      \
    _RAND_TEST_MESSAGE((_head), (_result))

        // Fast random integer.
        FastRandInitialize(rand_seed);
        _RAND_TEST(FastRand(), fast_rand_result, "FastRand(): ")

        // C standard random integer.
        srand(rand_seed);
        _RAND_TEST(rand(), std_rand_result, "C rand(): ")

        // C++ 11 STL random library.
        static default_random_engine engine;
        static uniform_int_distribution<> dist(0, RAND_MAX);
        engine.seed(rand_seed);
        _RAND_TEST(dist(engine), cpp11_rand_result, "C++ 11 random: ")

        Assert::IsTrue(std_rand_result == fast_rand_result);
    }

    TEST_METHOD(ArgTest)
    {
        PRINT_TEST_INFO("Arguments parser (argagg)")

        char * arg_exe_name = "test.exe";

        const auto arg_number = 123;
        const auto arg_option = "opt";

        char * argv_1[] = { arg_exe_name, "-n", "123", "-o", "opt", "-d" };
        char * argv_2[] = { arg_exe_name, "--number=123", "--option=opt", "--dumped" };
        char * argv_3[] = { arg_exe_name, "-d", "-n", "123", "--option=opt" };
        char * argv_4[] = { arg_exe_name, "-o", "opt", "--number=123" };

        auto argc_1 = static_cast<int>(size(argv_1));
        auto argc_2 = static_cast<int>(size(argv_2));
        auto argc_3 = static_cast<int>(size(argv_3));
        auto argc_4 = static_cast<int>(size(argv_4));

        argagg::parser arg_parser
            { {
                { "number", { "-n", "--number" }, "HELP MESSAGE FOR `number`", 1 },
                { "option", { "-o", "--option" }, "HELP MESSAGE FOR `option`", 1 },
                { "dumped", { "-d", "--dumped" }, "HELP MESSAGE FOR `dumped`", 0 }
            } };
        argagg::parser_results args;

#define _ARG_TEST(_argc, _argv, _is_arg_dumped)                             \
    args = arg_parser.parse((_argc), (_argv));                              \
    Assert::AreEqual(arg_number, static_cast<int>(args["number"]));         \
    Assert::AreEqual(arg_option, args["option"].as<string>("").c_str());    \
    Assert::AreEqual(_is_arg_dumped, static_cast<bool>(args["dumped"]));

        _ARG_TEST(argc_1, argv_1, true);
        _ARG_TEST(argc_2, argv_2, true);
        _ARG_TEST(argc_3, argv_3, true);
        _ARG_TEST(argc_4, argv_4, false);
    }

    TEST_METHOD(JsonTest)
    {
        PRINT_TEST_INFO("JSON parser (rapidjson)")

        string json_str =
            R"(
                {
                    "size": 23,
                    "pi": 3.1416,
                    "happy" : true,
                    "name" : "Alice",
                    "nothing" : null,
                    "list" : [1, 0, 2],
                    "answer" : { "everything": 42 },
                    "object" :
                        {
                            "currency": "USD",
                            "value" : 42.99
                        }
                }
            )";

        rapidjson::Document doc;
        doc.Parse(json_str.c_str());

        // Basic types.
        Assert::AreEqual(23,      doc["size"].GetInt());
        Assert::AreEqual(3.1416,  doc["pi"].GetDouble());
        Assert::AreEqual(true,    doc["happy"].GetBool());
        Assert::AreEqual("Alice", doc["name"].GetString());

        // Array.
        auto list = doc["list"].GetArray();
        vector<int> v;
        for (auto & i : list)
            v.push_back(i.GetInt());
        Assert::IsTrue(vector<int>{1, 0, 2} == v);

        // Object.
        Assert::AreEqual(42,    doc["answer"]["everything"].GetInt());
        Assert::AreEqual("USD", doc["object"]["currency"].GetString());
        Assert::AreEqual(42.99, doc["object"]["value"].GetDouble());
    }
};

ISING_TEST_NAMESPACE_END
