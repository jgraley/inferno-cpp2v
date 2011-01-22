/*
 * slave_test.cpp
 */

#include "steps/slave_test.hpp"
#include "tree/tree.hpp"
#include "common/common.hpp"
#include "helpers/soft_patterns.hpp"

// Just an early test for slaves, not a valid transformation
SlaveTest::SlaveTest()
{
    MakeTreePtr<For> s_for;
    MakeTreePtr<Statement> s_body;

    s_for->body = s_body;

    MakeTreePtr<Compound> r_comp;
    MakeTreePtr<Statement> r_body;
    MakeTreePtr<Continue> ss_cont;
    MakeTreePtr<Break> sr_break;
    MakeTreePtr< SlaveCompareReplace<Statement> > r_slave( r_body, ss_cont, sr_break );
    r_comp->statements = ( r_slave );

   	SearchReplace( s_for, r_comp );
}
