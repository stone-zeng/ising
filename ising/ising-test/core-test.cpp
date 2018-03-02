#include "stdafx.h"

// #include <cstdlib>
// #include <vector>
// #include <string>
#include "../ising-core/ising-2d.h"

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Ising::Test
{
    TEST_CLASS(CoreTest)
    {
    public:
        TEST_METHOD(PBCInitialize)
        {
            const size_t lattice_size = 10;
            Ising2D_PBC ising(lattice_size, lattice_size);
            ising.initialize();
        }

        //TEST_METHOD(InitializeFBC)
        //{}
    };
}
