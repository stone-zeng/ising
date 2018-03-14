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

        const double double_tolerance = 1.0e-6;

        const string file_path = ISING_SOLUTION_DIRECTORY;
        const string file_name = "ising-parameter-test.json";
        Parameter param(file_path + file_name);
        param.Parse();

        // Expected values.
        const LatticeSize size{ 10, 10 };
        const size_t beta_list_size = 20;
        vector<double> beta_list(beta_list_size);
        for (auto i = 0; i != beta_list_size; ++i)
            beta_list[i] = 10.0 / static_cast<double>(i + 1);
        const vector<double> magnetic_h_list = { 0.0 };
        const size_t iterations  = 200;
        const size_t n_ensemble  = 20;
        const size_t n_delta     = 3;
        const size_t repetitions = 1;

        Assert::IsTrue(size == param.lattice_size);
        Assert::IsTrue(kFree == param.boundary_condition);
        for(auto i = 0; i != beta_list.size(); ++i)
            Assert::AreEqual(beta_list[i], param.beta_list[i], double_tolerance);
        for (auto i = 0; i != magnetic_h_list.size(); ++i)
            Assert::AreEqual(magnetic_h_list[i], param.magnetic_h_list[i], double_tolerance);
        Assert::AreEqual(iterations,  param.iterations);
        Assert::AreEqual(n_ensemble,  param.n_ensemble);
        Assert::AreEqual(n_delta,     param.n_delta);
        Assert::AreEqual(repetitions, param.repetitions);
    }

    TEST_METHOD(PbcInitialize)
    {
        PRINT_TEST_INFO("Ising lattice initialization (PBC)")

        Ising2D_PBC s(lattice_size_, lattice_size_);
        s.Initialize();
        // Expected to be all one.
        _WriteLatticeMessagePBC(s);
    }

    TEST_METHOD(FbcInitialize)
    {
        PRINT_TEST_INFO("Ising lattice initialization (FBC)")

        Ising2D_FBC s(lattice_size_, lattice_size_);
        s.Initialize();
        // Expected to be all one excluding boundary.
        _WriteLatticeMessageFBC(s);
    }

    TEST_METHOD(PbcSweep)
    {
        PRINT_TEST_INFO("Ising lattice sweep (PBC)")

        Ising2D_PBC s(lattice_size_, lattice_size_);
        s.Initialize();
        s.Sweep(beta_, h_);
        _WriteLatticeMessagePBC(s);
    }

    TEST_METHOD(FbcSweep)
    {
        PRINT_TEST_INFO("Ising lattice sweep (FBC)")

        Ising2D_FBC s(lattice_size_, lattice_size_);
        s.Initialize();
        s.Sweep(beta_, h_);
        _WriteLatticeMessageFBC(s);
    }

    TEST_METHOD(PbcEvaluate)
    {
        PRINT_TEST_INFO("Ising lattice evaluate (PBC)")

        Ising2D_PBC s(lattice_size_, lattice_size_);
        s.Initialize();
        _WriteResultMessage(s.Evaluate(beta_, h_, iterations_, n_ensemble_));
        _WriteLatticeMessagePBC(s);
    }

    TEST_METHOD(FbcEvaluate)
    {
        PRINT_TEST_INFO("Ising lattice evaluate (FBC)")

        Ising2D_FBC s(lattice_size_, lattice_size_);
        s.Initialize();
        _WriteResultMessage(s.Evaluate(beta_, h_, iterations_, n_ensemble_));
        _WriteLatticeMessageFBC(s);
    }

private:
    template<typename T>
    void _WriteRowMessage(const T & s, const size_t & index)
    {
        auto row = "Line " + to_string(index + 1) + ":\t" + s.ShowRow(index);
        Logger::WriteMessage(row.c_str());
    }

    void _WriteLatticeMessagePBC(const Ising2D_PBC & s)
    {
        for (auto i = 0; i != lattice_size_; ++i)
            _WriteRowMessage(s, i);
    }

    void _WriteLatticeMessageFBC(const Ising2D_FBC & s)
    {
        for (auto i = 0; i != lattice_size_ + 2; ++i)
            _WriteRowMessage(s, i);
    }

    void _WriteResultMessage(const Observable & result)
    {
        auto m_str = "M = " + to_string(result.magnetic_dipole);
        auto e_str = "E = " + to_string(result.energy);
        Logger::WriteMessage(m_str.c_str());
        Logger::WriteMessage(e_str.c_str());
    }

    // Parameters for `Initialize()`, `Sweep()` and `Evaluation()` test.
    // Not used for `ParameterParse`.
    const size_t lattice_size_ = 6;
    const double beta_         = 0.44;  // Near critical point.
    const double h_            = 0.1;
    const size_t iterations_   = 100;
    const size_t n_ensemble_   = 20;
};

ISING_TEST_NAMESPACE_END
