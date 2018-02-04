#include "stdafx.h"
#include "CppUnitTest.h"

#include <string>
#include "../json.h"

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace isingtest
{
    TEST_CLASS(UnitTest1)
    {
    public:

        TEST_METHOD(TestJSON)
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
