/*
 * slave_test.cpp
 */

#include "steps/slave_test.hpp"
#include "tree/cpptree.hpp"
#include "common/common.hpp"
#include "sr/agents/all.hpp"

using namespace CPPTree;
using namespace Steps;

// Just an early test for slaves, not a valid transformation
SlaveTest::SlaveTest()
{
    MakePatternPtr<For> s_for;
    MakePatternPtr<Statement> s_body;

    s_for->body = s_body;

    MakePatternPtr<Compound> r_comp;
    MakePatternPtr<Statement> r_body;
    MakePatternPtr<Continue> ss_cont;
    MakePatternPtr<Break> sr_break;
    MakePatternPtr< SlaveCompareReplace<Statement> > r_slave( r_body, ss_cont, sr_break );
    r_comp->statements = ( r_slave );

   	SearchReplace( s_for, r_comp );
}
