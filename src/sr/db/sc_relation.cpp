#include "sc_relation.hpp"

#include "relation_test.hpp"

#include "helpers/simple_compare.hpp"
#include "helpers/simple_duplicate.hpp"

#include <random>

using namespace SR;


SimpleCompareRelation::SimpleCompareRelation() :
    simple_compare( make_shared<SimpleCompare>() )
{
}


bool SimpleCompareRelation::operator()( KeyType l_key, KeyType r_key ) const
{
    return Compare3Way(l_key, r_key) < 0;
}


Orderable::Diff SimpleCompareRelation::Compare3Way( KeyType l_key, KeyType r_key ) const
{
#ifdef SC_KEY_IS_NODE
    TreePtr<Node> l_node = l_key;
    TreePtr<Node> r_node = r_key;    
#else    
    TreePtr<Node> l_node = l_key.GetChildTreePtr();
    TreePtr<Node> r_node = r_key.GetChildTreePtr();    
#endif    

    auto l_minimax = TreePtr<MinimaxNode>::DynamicCast( l_node );
    if( l_minimax )
		l_node = l_minimax->GetGuide();
		
    auto r_minimax = TreePtr<MinimaxNode>::DynamicCast( r_node );
    if( r_minimax )
		r_node = r_minimax->GetGuide();

    if( Orderable::Diff d = simple_compare->Compare3Way( l_node, r_node ) )
		return d;

    if( !l_minimax && !r_minimax )
    {
#ifdef SC_KEY_IS_NODE
		return TreePtr<Node>::Compare3Way( l_key, r_key );
#else		
        return XLink::Compare3Way(l_key, r_key);
#endif  		
	}
    else if( l_minimax && r_minimax )
    {
		return (int)l_minimax->GetRole() - (int)r_minimax->GetRole();        
	}	
    else if( l_minimax && !r_minimax )
    {
		return (int)l_minimax->GetRole();        
	}	
    else if( !l_minimax && r_minimax )
    {
		return - (int)r_minimax->GetRole();        
	}	
	else
    {
		ASSERTFAIL();
	}
}


void SimpleCompareRelation::Test( const vector<KeyType> &keys )
{
	using namespace std::placeholders;

	TestRelationProperties<KeyType>( keys,
									 true,
									 "SimpleCompareRelation",
									 bind(&SimpleCompareRelation::Compare3Way, *this, _1, _2), 
    [&](KeyType l, KeyType r) -> bool
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
