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
    auto s_for = MakePatternPtr<For>();
    auto s_body = MakePatternPtr<Statement>();

    s_for->body = s_body;

    auto r_comp = MakePatternPtr<Compound>();
    auto r_body = MakePatternPtr<Statement>();
    auto ss_cont = MakePatternPtr<Continue>();
    auto sr_break = MakePatternPtr<Break>();
    auto r_slave = MakePatternPtr< SlaveCompareReplace<Statement> >( r_body, ss_cont, sr_break );
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
    
    auto t = MakePatternPtr<Something>();
    auto ss = MakePatternPtr<Something>();
    auto sr = MakePatternPtr<Something>();
    auto r_whenever = MakePatternPtr<Whenever>();
    auto r_slave = MakePatternPtr< SlaveCompareReplace<Node> >( t, ss, sr );
    auto s = MakePatternPtr<Something>();

    r_whenever->members = (r_slave, sr);
    
    Configure( COMPARE_REPLACE, s, r_whenever );
}


SlaveTest3::SlaveTest3() 
{    
    // In LATER slave S/R, r_whatever could invalidate r_slave2's root position
    // if it does eg builder stuff on the unwind. See #370, #378
    
    auto ss = MakePatternPtr<Something>();
    auto sr = MakePatternPtr<Something>();
    auto r_whatever = MakePatternPtr<Whatever>();
    auto r_slave1 = MakePatternPtr< SlaveCompareReplace<Node> >( r_whatever, ss, sr );

    auto s2s = MakePatternPtr<Something>();
    auto s2r = MakePatternPtr<Something>();
    auto r = MakePatternPtr<Something>();
    auto r_slave2 = MakePatternPtr< SlaveCompareReplace<Node> >( r, s2s, s2r );
    r_whatever->child = r_slave2;
    
    Configure( COMPARE_REPLACE, r_slave1 );
}

