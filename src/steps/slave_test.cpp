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

    Configure( SEARCH_REPLACE, s_for, r_comp );
}


struct Something : virtual Node 
{ 
    NODE_FUNCTIONS 
    virtual string GetColour() const { return "/set28/1"; }
};


struct Whatever : virtual Node 
{ 
    NODE_FUNCTIONS 
    TreePtr<Node> child; ///< is the instance constant (ie compile time value)?
    
    virtual string GetColour() const { return "/set28/2"; }
};


SlaveTest2::SlaveTest2()
{    
    MakePatternPtr<Something> ss;
    MakePatternPtr<Something> sr;
    MakePatternPtr<Whatever> r_whatever;
    MakePatternPtr< SlaveCompareReplace<Node> > r_slave1( r_whatever, ss, sr );

    MakePatternPtr<Something> s2s;
    MakePatternPtr<Something> s2r;
    MakePatternPtr<Something> r;
    MakePatternPtr< SlaveCompareReplace<Node> > r_slave2( r, s2s, s2r );
    r_whatever->child = r_slave2;
    
    Configure( COMPARE_REPLACE, r_slave1 );
}
