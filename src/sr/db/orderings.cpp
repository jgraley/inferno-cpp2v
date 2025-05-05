#include "orderings.hpp"

#include "x_tree_database.hpp"
#include "sc_relation.hpp"
#include "lacing.hpp"
#include "relation_test.hpp"


using namespace SR;    

//#define TRACE_CATEGORY_RELATION

//#define SC_INTRINSIC

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

	
void Orderings::MainTreeInsertGeometric(TreeZone *zone, const DBCommon::CoreInfo *base_info)
{     
	node_reached_count.clear();

    DBWalk::Actions actions;
    actions.push_back( bind(&Orderings::InsertGeometricAction, this, placeholders::_1) );
    db_walker.WalkTreeZone( &actions, zone, DBCommon::TreeOrdinal::MAIN, DBWalk::WIND_IN, base_info );

	// We may now re-instate SimpleCompare index entries for parents 
	// of the base node so that the SC ordering is intact. Base is
	// sufficient: what is ancestor of base is ancestor of every node in
	// the zone. If we act at root, there won't be any.
#ifdef SC_INTRINSIC
	set<TreePtr<Node>> invalidated_xlinks = GetTerminusAndBaseAncestors(*zone);
	for( TreePtr<Node> x : invalidated_xlinks )                        
	{
		// If it's in at this point it came from instrinsic and will be wrong due to placeholders
		if( simple_compare_ordering.count(x)!=0 )
			EraseSolo( simple_compare_ordering, x );                            
		InsertSolo( simple_compare_ordering, x );                           	
	}
#else
	// Assume there is only one incoming XLink to the node because not a leaf
	XLink x = zone->GetBaseXLink();
	while( x = db->TryGetParentXLink(x) )
		InsertSolo( simple_compare_ordering, x.GetChildTreePtr() );           
#endif	
}


void Orderings::MainTreeDeleteGeometric(TreeZone *zone, const DBCommon::CoreInfo *base_info)
{
	node_reached_count.clear();
	
	DBWalk::Actions actions;
    actions.push_back( bind(&Orderings::DeleteGeometricAction, this, placeholders::_1) );
    db_walker.WalkTreeZone( &actions, zone, DBCommon::TreeOrdinal::MAIN, DBWalk::WIND_OUT, base_info );

	// We must delete SimpleCompare index entries for ancestors of the base
	// node, since removing it will invalidate the SC ordering. Base is
	// sufficient: what is ancestor of base is ancestor of every node in
	// the zone. If we act at root, there won't be any.
#ifdef SC_INTRINSIC
	set<TreePtr<Node>> invalidated_xlinks = GetTerminusAndBaseAncestors(*zone);
	for( TreePtr<Node> x : invalidated_xlinks )
		EraseSolo( simple_compare_ordering, x );                              
#else
		// Assume there was only one incoming XLink to the node because not a leaf
	XLink x = zone->GetBaseXLink();
	while( x = db->TryGetParentXLink(x) )
		EraseSolo( simple_compare_ordering, x.GetChildTreePtr() );                              
#endif
}


void Orderings::InsertIntrinsic(FreeZone *zone)
{
	node_reached_count.clear();

    DBWalk::Actions actions;
    actions.push_back( bind(&Orderings::InsertIntrinsicAction, this, placeholders::_1) );
    db_walker.WalkFreeZone( &actions, zone, DBWalk::WIND_OUT );
}


void Orderings::DeleteIntrinsic(FreeZone *zone)
{
	node_reached_count.clear();

    DBWalk::Actions actions;
    actions.push_back( bind(&Orderings::DeleteIntrinsicAction, this, placeholders::_1) );
    db_walker.WalkFreeZone( &actions, zone, DBWalk::WIND_OUT );
}


void Orderings::InsertGeometricAction(const DBWalk::WalkInfo &walk_info)
{ 
	InsertSolo( depth_first_ordering, walk_info.xlink );
	
#ifndef SC_INTRINSIC
	// Intrinsic orderings are keyed on nodes, and we don't need to update on the boundary layer
	if( !walk_info.at_terminus )
	{        
		// Only if not already
		if( simple_compare_ordering.count(walk_info.node)==0 )
			InsertSolo( simple_compare_ordering, walk_info.node );               
	}
#endif	
}


void Orderings::DeleteGeometricAction(const DBWalk::WalkInfo &walk_info)
{		
	EraseSolo( depth_first_ordering, walk_info.xlink );

#ifndef SC_INTRINSIC
	// Intrinsic orderings are keyed on nodes, and we don't need to update on the boundary layer
	if( !walk_info.at_terminus )
	{
		// Node table hasn't been updated yet, so node should be in there.
		NodeTable::Row row = db->GetNodeRow(walk_info.node); 
		ASSERT( row.incoming_xlinks.count(walk_info.xlink)==1 );
		
		// Track the number of times we've reached each node in current zone
		node_reached_count[walk_info.node]++;     

		// Only remove if this was the last incoming XLink to the node
		if( node_reached_count.at(walk_info.node) == row.incoming_xlinks.size() ) 
			EraseSolo( simple_compare_ordering, walk_info.node );               
	} 
#endif	
}

        
void Orderings::InsertIntrinsicAction(const DBWalk::WalkInfo &walk_info)
{ 	
	// Intrinsic orderings are keyed on nodes, and we don't need to update on the boundary layer
	// We don't get an XLink for root because it's a free zone walk
	// We also don't get called on terminii so at_terminus is always false

	// Restrict to one reaching for the zone TODO could walk do this?
	if( node_reached_count[walk_info.node]++ > 0 )
		return;

    // Identifiers duplicate to themselves and therefore old ones can end up in the "new" FZs.
    // Detect this by looking at current DB
	if( db->HasNodeRow(walk_info.node) ) // TODO use domain for clarity
		return; 

	// Moreover, the same identifier can appear more than once in the layout's new FZs. 
	// Deal with this by checking our own ordering.
	if( category_ordering.count(walk_info.node) > 0 )
		return; 
		
	// Only if not already
	InsertSolo( category_ordering, walk_info.node );            	

#ifdef SC_INTRINSIC
	// Intrinsic orderings are keyed on nodes, and we don't need to update on the boundary layer
	// Only if not already
	InsertSolo( simple_compare_ordering, walk_info.node );               
#endif		
}


void Orderings::DeleteIntrinsicAction(const DBWalk::WalkInfo &walk_info)
{		
	// Intrinsic orderings are keyed on nodes, and we don't need to update on the boundary layer
	// We don't get an XLink for root because it's a free zone walk
	// We also don't get called on terminii so at_terminus is always false

	// Restrict to one reaching for the zone TODO could walk do this?
	if( node_reached_count[walk_info.node]++ > 0 )
		return;

    // Node must have been removed from the current db
	if( db->HasNodeRow(walk_info.node) ) // TODO use domain for clarity
		return; 
		
	EraseSolo( category_ordering, walk_info.node );   
	
#ifdef SC_INTRINSIC
	simple_compare_ordering.erase( walk_info.node );               	 
#endif		
}


set<TreePtr<Node>> Orderings::GetTerminusAndBaseAncestors( const TreeZone &tz ) const
{
	set<TreePtr<Node>> sn;
	set<XLink> sx;
		
	// Include parent of base all the way back to root
	XLink x = tz.GetBaseXLink();
	while(x = db->TryGetParentXLink(x))
	{
		InsertSolo(sx, x);			
		sn.insert(x.GetChildTreePtr());			
	}

	// Now for each terminus, include parent of terminus back to anything we 
	// already included.
	for( size_t i=0; i<tz.GetNumTerminii(); i++ )
	{
		x = tz.GetTerminusXLink(i);
		while(x = db->TryGetParentXLink(x))
		{
			if( sx.count(x)!=0 )
				break; // There will already be a path to root from here
			InsertSolo(sx, x);				
			sn.insert(x.GetChildTreePtr());			
		}
	}
	
	return sn;
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
		CheckEqualOrdering( "SC", l->simple_compare_ordering, r->simple_compare_ordering );
	}
	else
	{
		CheckEqualOrdering( "DF", l->depth_first_ordering, r->depth_first_ordering );
	}
}
