#include "orderings.hpp"

#include "x_tree_database.hpp"
#include "sc_relation.hpp"
#include "lacing.hpp"
#include "relation_test.hpp"

using namespace SR;   

Orderings::Orderings( shared_ptr<Lacing> lacing, const XTreeDatabase *db_ ) :
    plan( lacing ),
    depth_first_ordering( db_ ),
    category_ordering( plan.lacing ),
    db( db_ )
{ 
}


Orderings::Plan::Plan( shared_ptr<Lacing> lacing_ ) :
    lacing( lacing_ )
{
}    


const Lacing *Orderings::GetLacing() const
{
    return plan.lacing.get();
}

	
void Orderings::InsertTree(TreeZone &zone)
{     
	// -------------------- depth-first -----------------------
    InsertGeometric(zone);
	
	// -------------------- simple compare and category -----------------------
	// SC and CAT are node-keyed. CAT is pure intrinsic, but SC has additionals

	// Use a walk to insert all the SC and CAT when intrinsic
	db_walker.WalkTreeZone( bind(&Orderings::InsertActionSCAndCAT, this, placeholders::_1), 
	                        zone, DBWalk::WIND_IN );

	// SC still needs ancestors of root
	auto subtree = TreeZone::CreateSubtree(zone.GetBaseXLink());

	// GetTerminusAndBaseAncestors() never gives us a leaf node, so no need to check for other parents
	for( TreePtr<Node> x : GetTerminusAndBaseAncestors(subtree) )                        
		InsertSolo( simple_compare_ordering, x );   			
}


void Orderings::DeleteTree(TreeZone &zone)
{
	// -------------------- depth-first -----------------------
    DeleteGeometric(zone);

	// -------------------- simple compare and category -----------------------
	// SC and CAT are node-keyed. CAT is pure intrinsic, but SC has additionals.
	
	// Use a walk to insert all the SC and CAT when intrinsic
	node_reached_count.clear();
	db_walker.WalkTreeZone( bind(&Orderings::DeleteActionSCAndCAT, this, placeholders::_1),
	                        zone, DBWalk::WIND_OUT );

	// SC still needs ancestors of root
	auto subtree = TreeZone::CreateSubtree(zone.GetBaseXLink());
	
	// GetTerminusAndBaseAncestors() never gives us a leaf node, so no need to check for other parents
	for( TreePtr<Node> x : GetTerminusAndBaseAncestors(subtree) )    
		EraseSolo( simple_compare_ordering, x );                              
}


Orderings::SwapTransaction::SwapTransaction(Orderings *orderings_, TreeZone &zone1_, TreeZone &zone2_ ) :
	DBCommon::SwapTransaction( zone1_, zone2_ ),
	orderings( *orderings_ )
{	
	// -------------------- depth-first -----------------------
    orderings.DeleteGeometric(zone1);
    orderings.DeleteGeometric(zone2);

	// -------------------- simple compare -----------------------
	for( TreePtr<Node> x : orderings.GetTerminusAndBaseAncestors(zone1) )    
		EraseSolo( orderings.simple_compare_ordering, x );                              
	for( TreePtr<Node> x : orderings.GetTerminusAndBaseAncestors(zone2) )    
		EraseSolo( orderings.simple_compare_ordering, x );                              
}


Orderings::SwapTransaction::~SwapTransaction()
{
	// -------------------- depth-first -----------------------
    orderings.InsertGeometric(zone1);
    orderings.InsertGeometric(zone2);

	// -------------------- simple compare -----------------------
	for( TreePtr<Node> x : orderings.GetTerminusAndBaseAncestors(zone1) )                        
		InsertSolo( orderings.simple_compare_ordering, x );   			
	for( TreePtr<Node> x : orderings.GetTerminusAndBaseAncestors(zone2) )                        
		InsertSolo( orderings.simple_compare_ordering, x );   			
}

        
void Orderings::InsertGeometric(const TreeZone &zone)
{     
	// Take care of the DFO, which is an XLink-keyed ordering and must be updated fully in geom case
    auto action = [&](const DBWalk::WalkInfo &walk_info)
    {
	 	InsertSolo( depth_first_ordering, walk_info.xlink );
	};
	
    db_walker.WalkTreeZone( action, zone, DBWalk::WIND_IN );
}


void Orderings::DeleteGeometric(const TreeZone &zone)
{
	// Take care of the DFO, which is an XLink-keyed ordering and must be updated fully in geom case
    auto action = [&](const DBWalk::WalkInfo &walk_info)
    {
	 	EraseSolo( depth_first_ordering, walk_info.xlink );
	};
	
    db_walker.WalkTreeZone( action, zone, DBWalk::WIND_IN );
}


void Orderings::InsertActionSCAndCAT(const DBWalk::WalkInfo &walk_info)
{ 
	// Remaining orderings are keyed on nodes, and we don't need to update on the boundary layer
	if( walk_info.at_terminus )
		return;

	// Multiple parents: only if not already
	if( simple_compare_ordering.count(walk_info.node)==0 )
		InsertSolo( simple_compare_ordering, walk_info.node );               

	// Multiple parents: only if not already
	if( category_ordering.count(walk_info.node) == 0 )
	{
		TRACE("CAT inserts: ")(walk_info.node)("\n");
		InsertSolo( category_ordering, walk_info.node );            	
	}
}


void Orderings::DeleteActionSCAndCAT(const DBWalk::WalkInfo &walk_info)
{			
	// Remaining orderings are keyed on nodes, and we don't need to update on the boundary layer
	if( walk_info.at_terminus )
		return;

	// Multiple parents: only remove if this was the last incoming XLink to the node
	// If we reached all the incomers so there are none outside the zone, skip		
	if( db->GetNodeRow(walk_info.node).incoming_xlinks.size() == node_reached_count[walk_info.node]+1 ) 
	{		
		EraseSolo( simple_compare_ordering, walk_info.node );               
	
		TRACE("CAT deletes: ")(walk_info.node)("\n");
		EraseSolo( category_ordering, walk_info.node );   
		TRACE("CAT at %p size=%u\n", this, category_ordering.size());	
    }

	// Track the number of times we've reached each node in current zone
	node_reached_count[walk_info.node]++;     
}
       

set<TreePtr<Node>> Orderings::GetTerminusAndBaseAncestors( const TreeZone &tz ) const
{
	set<TreePtr<Node>> sn;
		
	// Include parent of base all the way back to root
	XLink x = tz.GetBaseXLink();
	while(x = db->TryGetParentXLink(x))
	{
		InsertSolo(sn, x.GetChildTreePtr());			
	}

	// Now for each terminus, include parent of terminus back to anything we 
	// already included. As a result, we'll never produce a leaf node, so no need to 
	// deal with multiple parents.
	for( size_t i=0; i<tz.GetNumTerminii(); i++ )
	{
		x = tz.GetTerminusXLink(i);
		while(x = db->TryGetParentXLink(x))
		{
			if( sn.count(x.GetChildTreePtr())!=0 )
				break; // There will already be a path to root from here
			InsertSolo(sn, x.GetChildTreePtr());			
		}
	}
	
	return sn;
}


void Orderings::CheckSizeIs( size_t tot_num_xlinks, size_t tot_num_nodes ) const
{
	ASSERT( depth_first_ordering.size() == tot_num_xlinks );
	ASSERT( category_ordering.size() == tot_num_nodes );
	ASSERT( simple_compare_ordering.size() == tot_num_nodes );
}


void Orderings::Dump() const
{
    TRACE("category_ordering:\n")(category_ordering)("\n");
}


void Orderings::CheckRelations( const vector<XLink> &xlink_domain,  
                                const vector<TreePtr<Node>> &node_domain )
{
    SimpleCompareRelation scr;
    scr.Test( node_domain );

    TestOrderingIntact( simple_compare_ordering,
                        true,
                        "simple_compare_ordering" );

    CategoryRelation cat_r( plan.lacing );
    cat_r.Test( node_domain );    
    
    TestOrderingIntact( category_ordering,
                        true,
                        "category_ordering" );

    DepthFirstRelation dfr( db );
    dfr.Test( xlink_domain );    

    TestOrderingIntact( depth_first_ordering,
                        true,
                        "depth_first_ordering" );
}


template<typename ORDERING>
void CheckEqualOrdering( string name, const ORDERING &l, const ORDERING &r  )
{
    auto lk = KeysToSet(l);
    auto rk = KeysToSet(r);
    ASSERT(IsEquivalent(lk, rk))(name)(" ordering mismatch:\n")(DiffTrace(lk, rk))("\nReference ordering:\n")(l);
}


void Orderings::CheckEqual( shared_ptr<Orderings> l, shared_ptr<Orderings> r, bool intrinsic )
{
	if( intrinsic )
	{
		CheckEqualOrdering( "CAT", l->category_ordering, r->category_ordering );
	}
	else
	{
		CheckEqualOrdering( "SC", l->simple_compare_ordering, r->simple_compare_ordering );
		CheckEqualOrdering( "DF", l->depth_first_ordering, r->depth_first_ordering );
	}
}
