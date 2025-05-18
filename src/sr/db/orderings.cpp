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

	
void Orderings::Insert(TreeZone &zone, const DBCommon::CoreInfo *base_info, bool do_intrinsics)
{     
	node_reached_count.clear();

    DBWalk::Actions actions;
    actions.push_back( bind(&Orderings::InsertAction, this, placeholders::_1, do_intrinsics) );
    db_walker.WalkTreeZone( &actions, zone, DBCommon::TreeOrdinal(-1), DBWalk::WIND_IN, base_info );

	// We may now re-instate SimpleCompare index entries for parents 
	// of the base node so that the SC ordering is intact. Base is
	// sufficient: what is ancestor of base is ancestor of every node in
	// the zone. If we act at root, there won't be any.

	// Assume there is only one incoming XLink to the node because not a leaf
	auto subtree = XTreeZone::CreateSubtree(zone.GetBaseXLink());
	set<TreePtr<Node>> invalidated = GetTerminusAndBaseAncestors(*subtree);

	for( TreePtr<Node> x : invalidated )                        
		if( !dynamic_cast<ScaffoldBase *>(x.get()) )
			InsertSolo( simple_compare_ordering, x );   			
}


void Orderings::Delete(TreeZone &zone, const DBCommon::CoreInfo *base_info, bool do_intrinsics)
{
	node_reached_count.clear();
	
	DBWalk::Actions actions;
    actions.push_back( bind(&Orderings::DeleteAction, this, placeholders::_1, do_intrinsics) );
    db_walker.WalkTreeZone( &actions, zone, DBCommon::TreeOrdinal(-1), DBWalk::WIND_OUT, base_info );

	// We must delete SimpleCompare index entries for ancestors of the base
	// node, since removing it will invalidate the SC ordering. Base is
	// sufficient: what is ancestor of base is ancestor of every node in
	// the zone. If we act at root, there won't be any.

	// Assume there was only one incoming XLink to the node because not a leaf
	auto subtree = XTreeZone::CreateSubtree(zone.GetBaseXLink());
	set<TreePtr<Node>> invalidated = GetTerminusAndBaseAncestors(*subtree);

	for( TreePtr<Node> x : invalidated )                        
		if( !dynamic_cast<ScaffoldBase *>(x.get()) )
			EraseSolo( simple_compare_ordering, x );                              
}


void Orderings::InsertAction(const DBWalk::WalkInfo &walk_info, bool do_intrinsics)
{ 
	InsertSolo( depth_first_ordering, walk_info.xlink );

	// Remaining orderings are keyed on nodes, and we don't need to update on the boundary layer
	if( walk_info.at_terminus )
		return;

	if( dynamic_cast<ScaffoldBase *>(walk_info.node.get()) )
		return;

	// Only if not already
	if( simple_compare_ordering.count(walk_info.node)==0 )
		InsertSolo( simple_compare_ordering, walk_info.node );               

	if( do_intrinsics )
	{		
		//ASSERT( (node_ref_counts[walk_info.node] == 0) ==
		//        (category_ordering.count(walk_info.node)==0) );
		
		if( node_ref_counts[walk_info.node]++ != 0 )
		{
			TRACE("CAT does not insert due existing refs: ")(walk_info.node)(" count=%u\n", node_ref_counts[walk_info.node]);
			return;
		}		 
		
		TRACE("CAT inserts: ")(walk_info.node)("\n");
		InsertSolo( category_ordering, walk_info.node );            	
		TRACE("CAT at %p size=%u count=%u\n", this, category_ordering.size(), node_ref_counts[walk_info.node]);		
	}
}


void Orderings::DeleteAction(const DBWalk::WalkInfo &walk_info, bool do_intrinsics)
{		
	EraseSolo( depth_first_ordering, walk_info.xlink );

	// Remaining orderings are keyed on nodes, and we don't need to update on the boundary layer
	if( walk_info.at_terminus )
		return;

	if( dynamic_cast<ScaffoldBase *>(walk_info.node.get()) )
		return;

	// Node table hasn't been updated yet, so node should be in there.
	NodeTable::Row row = db->GetNodeRow(walk_info.node); 
	ASSERT( row.incoming_xlinks.count(walk_info.xlink)==1 );
		
	// Track the number of times we've reached each node in current zone
	node_reached_count[walk_info.node]++;     

	// Only remove if this was the last incoming XLink to the node
	if( node_reached_count.at(walk_info.node) == row.incoming_xlinks.size() ) 
		EraseSolo( simple_compare_ordering, walk_info.node );               
	
	if( do_intrinsics )
	{		
		/*ASSERT( (node_reached_count.at(walk_info.node) == row.incoming_xlinks.size()) ==
		        (node_ref_counts[walk_info.node] == 1) )
		        ("Node: ")(walk_info.node)(" XLink: ")(walk_info.xlink)("\n")
		        ("node_reached_count: ")(node_reached_count.at(walk_info.node))("\n")
		        ("row.incoming_xlinks: ")(row.incoming_xlinks)("\n")
		        ("node_ref_count: ")(node_ref_counts[walk_info.node])("\n");
		  */      
		
		if( --node_ref_counts[walk_info.node] != 0 )
		{
			TRACE("CAT does not delete due existing refs: ")(walk_info.node)("\n");
			return;
		}	
			
		TRACE("CAT deletes: ")(walk_info.node)("\n");
		EraseSolo( category_ordering, walk_info.node );   
		TRACE("CAT at %p size=%u\n", this, category_ordering.size());	
	}
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
	}
	else
	{
		CheckEqualOrdering( "SC", l->simple_compare_ordering, r->simple_compare_ordering );
		CheckEqualOrdering( "DF", l->depth_first_ordering, r->depth_first_ordering );
	}
}
