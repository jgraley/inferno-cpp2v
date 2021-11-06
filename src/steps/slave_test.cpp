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
    virtual string GetColour() const { return "/set19/1"; }
};


struct Whatever : virtual Node 
{ 
    NODE_FUNCTIONS 
    TreePtr<Node> child; 
    
    virtual string GetColour() const { return "/set19/2"; }
};


struct Whenever : virtual Node 
{ 
    NODE_FUNCTIONS 
    Sequence<Node> members;
    
    virtual string GetColour() const { return "/set19/3"; }
};


SlaveTest2::SlaveTest2() 
{    
    // Ambiguity with SOONER slave S/R: is sr keyed by r_slave
    // or master? Depends whether slave hits. See #370.
    
    MakePatternPtr<Something> t;
    MakePatternPtr<Something> ss;
    MakePatternPtr<Something> sr;
    MakePatternPtr<Whenever> r_whenever;
    MakePatternPtr< SlaveCompareReplace<Node> > r_slave( t, ss, sr );
    MakePatternPtr<Something> s;

    r_whenever->members = (r_slave, sr);
    
    Configure( COMPARE_REPLACE, s, r_whenever );
}


SlaveTest3::SlaveTest3() 
{    
    // In LATER slave S/R, r_whatever could invalidate r_slave2's root position
    // if it does eg builder stuff on the unwind. See #370, #378
    
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

