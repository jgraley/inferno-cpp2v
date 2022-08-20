#include "unit_test.hpp"

#include "node/node.hpp"
#include "common/standard.hpp"
#include "sr/sym/truth_table.hpp"

#include <cstdlib>


void SelfTest()
{
    GenericsTest();
    SYM::TestTruthTable();
}

// TODO Consider multi-terminus Stuff and multi-root (StarStuff)

