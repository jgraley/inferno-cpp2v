#include "sc_relation.hpp"

#include "relation_test.hpp"

#include "helpers/simple_compare.hpp"
#include "helpers/duplicate.hpp"

#include <random>

using namespace SR;

//////////////////////////// SimpleCompareRelation ///////////////////////////////

SimpleCompareRelation::SimpleCompareRelation() :
    simple_compare( make_shared<SimpleCompare>() )
{
}


Orderable::Diff SimpleCompareRelation::Compare( XLink l_xlink, XLink r_xlink ) const
{
    // Get the child nodes and disregard the arrow heads
    TreePtr<Node> l_node = l_xlink.GetChildX();
    TreePtr<Node> r_node = r_xlink.GetChildX();
    
    // And then resort to SimpleCompare
    return simple_compare->Compare( l_node, r_node );
}


bool SimpleCompareRelation::operator()( XLink l_xlink, XLink r_xlink ) const
{
    return Compare(l_xlink, r_xlink) < 0;
}


void SimpleCompareRelation::Test( const unordered_set<XLink> &xlinks )
{
	using namespace std::placeholders;

	TestRelationProperties( xlinks,
                            false, // TODO require totality so can use set<> and not lose domain elements
                            "SimpleCompareRelation",
                            bind(&SimpleCompareRelation::Compare, *this, _1, _2), 
    [&](XLink l, XLink r)
    { 
        return l==r; 
    } );
}