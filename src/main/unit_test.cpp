#include "unit_test.hpp"

#include "node/node.hpp"
#include "common/standard.hpp"
#include "sr/sym/truth_table.hpp"

#include <cstdlib>


void SelfTest()
{
    CommonTest();
    GenericsTest();
    SYM::TestTruthTable();
}


// TODO Make Filter a functor. 
// TODO Consider merging Filter into Transformation.
// TODO Consider multi-terminus Stuff and multi-root (StarStuff)
