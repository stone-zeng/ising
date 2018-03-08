#include "stdafx.h"

#include "ising-core/fast-rand.h"
#include "ising-core/get-option.h"
#include "ising-core/win-timing.h"

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace rapidjson;
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

    TEST_METHOD(GetOptionTest)
    {
        PRINT_TEST_INFO("Get option")

        char * arg_exe_name = "test.exe";
        char * arg_n = "123";
        char * arg_o = "options";

        char * argv[] = { arg_exe_name, "-n", arg_n, "-o", arg_o };
        int argc = static_cast<int>(size(argv));

        GetOption options(argc, argv);

        Assert::AreEqual(atoi(arg_n), stoi(options.Parse('n')));
        Assert::AreEqual(string(arg_o), options.Parse('o'));
    }

    TEST_METHOD(JsonTest)
    {
        PRINT_TEST_INFO("Rapid JSON")

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

        Document doc;
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
