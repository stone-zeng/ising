#include "stdafx.h"

#include "ising-core/ising-2d.h"

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ising::Test
{

TEST_CLASS(CoreTest)
{
public:
    TEST_METHOD(PBCInitialize)
    {
        PRINT_TEST_INFO("Ising lattice initialization (PBC)")

        const size_t lattice_size = 10;
        Ising2D_PBC ising(lattice_size, lattice_size);
        ising.initialize();
    }

    TEST_METHOD(FBCInitialize)
    {
        PRINT_TEST_INFO("Ising lattice initialization (FBC)")

        const size_t lattice_size = 10;
        Ising2D_FBC ising(lattice_size, lattice_size);
        ising.initialize();
    }
};

}  // End of ising::test.
