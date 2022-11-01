#include "steps/test_steps.hpp"
#include "tree/cpptree.hpp"
#include "common/common.hpp"
#include "sr/agents/all.hpp"

using namespace CPPTree;
using namespace Steps;

// Just an early test for embedded patterns, not a valid transformation
EmbeddedSCRTest::EmbeddedSCRTest()
{
    auto s_for = MakePatternNode<CPPTree::For>();
    auto s_body = MakePatternNode<CPPTree::Statement>();

    s_for->body = s_body;

    auto r_comp = MakePatternNode<CPPTree::Compound>();
    auto r_body = MakePatternNode<CPPTree::Statement>();
    auto ss_cont = MakePatternNode<CPPTree::Continue>();
    auto sr_break = MakePatternNode<CPPTree::Break>();
    auto r_embedded = MakePatternNode< EmbeddedCompareReplace<CPPTree::Statement> >( r_body, ss_cont, sr_break );
    r_comp->statements = ( r_embedded );

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


EmbeddedSCRTest2::EmbeddedSCRTest2() 
{    
    // Ambiguity with SOONER model: is sr keyed by r_embedded
    // or enclosing? Depends whether embedded search hits. See #370.
    
    auto t = MakePatternNode<Something>();
    auto ss = MakePatternNode<Something>();
    auto sr = MakePatternNode<Something>();
    auto r_whenever = MakePatternNode<Whenever>();
    auto r_embedded = MakePatternNode< EmbeddedCompareReplace<Node> >( t, ss, sr );
    auto s = MakePatternNode<Something>();

    r_whenever->members = (r_embedded, sr);
    
    Configure( COMPARE_REPLACE, s, r_whenever );
}


EmbeddedSCRTest3::EmbeddedSCRTest3() 
{    
    // In LATER model, r_whatever could invalidate r_embedded_2's root position
    // if it does eg builder stuff on the unwind. See #370, #378
    
    auto ss = MakePatternNode<Something>();
    auto sr = MakePatternNode<Something>();
    auto r_whatever = MakePatternNode<Whatever>();
    auto r_embedded_1 = MakePatternNode< EmbeddedCompareReplace<Node> >( r_whatever, ss, sr );

    auto s2s = MakePatternNode<Something>();
    auto s2r = MakePatternNode<Something>();
    auto r = MakePatternNode<Something>();
    auto r_embedded_2 = MakePatternNode< EmbeddedCompareReplace<Node> >( r, s2s, s2r );
    r_whatever->child = r_embedded_2;
    
    Configure( COMPARE_REPLACE, r_embedded_1 );
}


FixCrazyNumber::FixCrazyNumber()
{
    // #576 should hit sctest04, which has been "fiddled".
    Configure( SEARCH_REPLACE, 
               MakePatternNode<SpecificInteger>(29258),
               MakePatternNode<SpecificInteger>(5) );
}


FixCrazyNumberEmb::FixCrazyNumberEmb()
{
    // The idea behind this test is to operate on test/examples/issue667.cpp and replace the
    // 54257 with a 50 _by the action of an embedded C&R_, since, at the time of writing, partial db
    // update only operates at embedded replace action. With current incremental tech, the tree up to 
    // and incuding the Assign "wants" to stay the same, and only the SpecificInteger "wants" to 
    // change. But the SimpleCompare relation between the Assign nodes will mutate due to the effect 
    // of the changing SpecificInteger value, and so the ordering will break if this is not catered for.
    auto ss = MakePatternNode<CPPTree::Assign>();
    auto sr = MakePatternNode<CPPTree::Assign>();

    auto se = MakePatternNode<CPPTree::Expression>();    
    auto r_embedded = MakePatternNode< EmbeddedCompareReplace<Expression> >( MakePatternNode<SpecificInteger>(54257), // through restricts to crazy number
                                                                             MakePatternNode<SpecificInteger>(54257), // emb matches for crazy 
                                                                             MakePatternNode<SpecificInteger>(50) ); // and replaces with sensible

    ss->operands = (se, r_embedded);
    sr->operands = (se, r_embedded);

    Configure( SEARCH_REPLACE, ss, sr );
}