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


Orderable::Diff SimpleCompareRelation::Compare3Way( XLink l_xlink, XLink r_xlink ) const
{
    TreePtr<Node> l_node = l_xlink.GetChildX();
    TreePtr<Node> r_node = r_xlink.GetChildX();    

    auto l_minimax = TreePtr<MinimaxNode>::DynamicCast( l_node );
    if( l_minimax )
		l_node = l_minimax->GetGuide();
		
    auto r_minimax = TreePtr<MinimaxNode>::DynamicCast( r_node );
    if( r_minimax )
		r_node = r_minimax->GetGuide();

    if( !l_minimax && !r_minimax )
    {
		// TODO fall back to XLink direct compare
        return simple_compare->Compare3Way( l_node, r_node );
	}
    else if( l_minimax && r_minimax )
    {
        if( Orderable::Diff d = simple_compare->Compare3Way( l_node, r_node ) )
			return d;
		return (int)l_minimax->GetRole() - (int)r_minimax->GetRole();        
	}	
    else if( l_minimax && !r_minimax )
    {
        if( Orderable::Diff d = simple_compare->Compare3Way( l_node, r_node ) )
			return d;
		return (int)l_minimax->GetRole();        
	}	
    else if( !l_minimax && r_minimax )
    {
        if( Orderable::Diff d = simple_compare->Compare3Way( l_node, r_node ) )
			return d;
		return - (int)r_minimax->GetRole();        
	}	
	else
    {
		ASSERTFAIL();
	}
}


bool SimpleCompareRelation::operator()( XLink l_xlink, XLink r_xlink ) const
{
    return Compare3Way(l_xlink, r_xlink) < 0;
}


void SimpleCompareRelation::Test( const unordered_set<XLink> &xlinks )
{
	using namespace std::placeholders;

	TestRelationProperties( xlinks,
                            false, // TODO require totality so can use set<> and not lose domain elements
                            "SimpleCompareRelation",
                            bind(&SimpleCompareRelation::Compare3Way, *this, _1, _2), 
    [&](XLink l, XLink r)
    { 
        return l==r; 
    } );
}


SimpleCompareRelation::MinimaxNode::MinimaxNode( TreePtr<Node> guide_, BoundingRole role_ ) :
	guide(guide_),
	role(role_)
{
}


SimpleCompareRelation::MinimaxNode::MinimaxNode() :
	guide(nullptr),
	role(Orderable::BoundingRole::NONE)
{
}


TreePtr<Node> SimpleCompareRelation::MinimaxNode::GetGuide() const
{
	return guide;
}


Orderable::BoundingRole SimpleCompareRelation::MinimaxNode::GetRole() const
{
	return role;
}


string SimpleCompareRelation::MinimaxNode::GetName() const
{
    return "SC-" + Orderable::RoleToString(role) + "(" + Trace(guide) + ")";
}
