#include "stdafx.h"

#include "ising-core/ising-parameter.h"
#include "ising-core/ising-2d.h"

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ising::Test
{

TEST_CLASS(CoreTest)
{
public:
    TEST_METHOD(ParameterParse)
    {
        PRINT_TEST_INFO("Ising parameters")

        const string file_path = ISING_SOLUTION_DIRECTORY;
        const string file_name = "ising-parameter-test.json";
        Parameter param(file_path + file_name);

        vector<double> beta_array{ 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0,
                                   1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2.0 };
        for (auto & i : beta_array)
            i = 1 / i;

        Assert::IsTrue(kFree == param.boundary_type_);
        // Due to the float point numbers' precision problem, I need to set tolerance = 1e-6.
        for(auto i = 0; i != beta_array.size(); ++i)
            Assert::AreEqual(beta_array[i], param.beta_list_[i], 1.0e-6);
    }

    TEST_METHOD(PBCInitialize)
    {
        PRINT_TEST_INFO("Ising lattice initialization (PBC)")

        const size_t lattice_size = 10;
        Ising2D_PBC ising(lattice_size, lattice_size);
        ising.Initialize();
    }

    TEST_METHOD(FBCInitialize)
    {
        PRINT_TEST_INFO("Ising lattice initialization (FBC)")

        const size_t lattice_size = 10;
        Ising2D_FBC ising(lattice_size, lattice_size);
        ising.Initialize();
    }
};

}  // End of ising::test.
