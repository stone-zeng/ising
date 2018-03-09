#include "stdafx.h"

#include "ising-core/ising-definitions.h"
#include "ising-core/ising-parameter.h"
#include "ising-core/ising-2d.h"

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

ISING_TEST_NAMESPACE_BEGIN

TEST_CLASS(CoreTest)
{
public:
    TEST_METHOD(ParameterParse)
    {
        PRINT_TEST_INFO("Ising parameters")

        double double_tolerance = 1.0e-6;

        string file_path = ISING_SOLUTION_DIRECTORY;
        string file_name = "ising-parameter-test.json";
        Parameter param(file_path + file_name);
        param.Parse();

        // Expected values.
        LatticeSize size{ 10, 10 };
        size_t beta_list_size = 20;
        vector<double> beta_list(beta_list_size);
        for (auto i = 0; i != beta_list_size; ++i)
            beta_list[i] = 10.0 / static_cast<double>(i + 1);
        vector<double> magnetic_h_list = { 0.0 };
        size_t iterations = 200;
        size_t n_ensemble = 20;
        size_t n_delta    = 3;

        Assert::IsTrue(size == param.lattice_size);
        Assert::IsTrue(kFree == param.boundary_condition);
        for(auto i = 0; i != beta_list.size(); ++i)
            Assert::AreEqual(beta_list[i], param.beta_list[i], double_tolerance);
        for (auto i = 0; i != magnetic_h_list.size(); ++i)
            Assert::AreEqual(magnetic_h_list[i], param.magnetic_h_list[i], double_tolerance);
        Assert::AreEqual(iterations, param.iterations);
        Assert::AreEqual(n_ensemble, param.n_ensemble);
        Assert::AreEqual(n_delta,    param.n_delta);
    }

    TEST_METHOD(PBCInitialize)
    {
        PRINT_TEST_INFO("Ising lattice initialization (PBC)")

        size_t lattice_size = 8;
        Ising2D_PBC ising(lattice_size, lattice_size);
        ising.Initialize();

        // Expected to be all zero.
        for (auto i = 0; i != lattice_size; ++i)
        {
            auto row = "Line " + to_string(i + 1) + ":\t" + ising.ShowRow(i);
            Logger::WriteMessage(row.c_str());
        }
    }

    TEST_METHOD(FBCInitialize)
    {
        PRINT_TEST_INFO("Ising lattice initialization (FBC)")

        size_t lattice_size = 8;
        Ising2D_FBC ising(lattice_size, lattice_size);
        ising.Initialize();

        // Expected to be all zero excluding boundary.
        for (auto i = 0; i != lattice_size + 2; ++i)
        {
            auto row = "Line " + to_string(i) + ":\t" + ising.ShowRow(i);
            Logger::WriteMessage(row.c_str());
        }
    }
};

ISING_TEST_NAMESPACE_END
