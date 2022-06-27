/*
 * slave_test.cpp
 */

#include "steps/test_steps.hpp"
#include "tree/cpptree.hpp"
#include "common/common.hpp"
#include "sr/agents/all.hpp"

using namespace CPPTree;
using namespace Steps;

// Just an early test for slaves, not a valid transformation
SlaveTest::SlaveTest()
{
    auto s_for = MakePatternNode<For>();
    auto s_body = MakePatternNode<Statement>();

    s_for->body = s_body;

    auto r_comp = MakePatternNode<Compound>();
    auto r_body = MakePatternNode<Statement>();
    auto ss_cont = MakePatternNode<Continue>();
    auto sr_break = MakePatternNode<Break>();
    auto r_slave = MakePatternNode< SlaveCompareReplace<Statement> >( r_body, ss_cont, sr_break );
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
    
    auto t = MakePatternNode<Something>();
    auto ss = MakePatternNode<Something>();
    auto sr = MakePatternNode<Something>();
    auto r_whenever = MakePatternNode<Whenever>();
    auto r_slave = MakePatternNode< SlaveCompareReplace<Node> >( t, ss, sr );
    auto s = MakePatternNode<Something>();

    r_whenever->members = (r_slave, sr);
    
    Configure( COMPARE_REPLACE, s, r_whenever );
}


SlaveTest3::SlaveTest3() 
{    
    // In LATER slave S/R, r_whatever could invalidate r_slave2's root position
    // if it does eg builder stuff on the unwind. See #370, #378
    
    auto ss = MakePatternNode<Something>();
    auto sr = MakePatternNode<Something>();
    auto r_whatever = MakePatternNode<Whatever>();
    auto r_slave1 = MakePatternNode< SlaveCompareReplace<Node> >( r_whatever, ss, sr );

    auto s2s = MakePatternNode<Something>();
    auto s2r = MakePatternNode<Something>();
    auto r = MakePatternNode<Something>();
    auto r_slave2 = MakePatternNode< SlaveCompareReplace<Node> >( r, s2s, s2r );
    r_whatever->child = r_slave2;
    
    Configure( COMPARE_REPLACE, r_slave1 );
}


FixCrazyNumber::FixCrazyNumber()
{
    // #576 should hit sctest04, which has been "fiddled".
    Configure( SEARCH_REPLACE, 
               MakePatternNode<SpecificInteger>(29258),
               MakePatternNode<SpecificInteger>(5) );
}