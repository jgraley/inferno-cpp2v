#include "orderings.hpp"

#include "x_tree_database.hpp"
#include "sc_relation.hpp"
#include "lacing.hpp"
#include "relation_test.hpp"


using namespace SR;    

//#define TRACE_CATEGORY_RELATION

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


DBWalk::Action Orderings::GetDeleteGeometricAction()
{            
    return [=](const DBWalk::WalkInfo &walk_info)
    {		
        EraseSolo( depth_first_ordering, walk_info.xlink );

        // Intrinsic orderings are keyed on nodes, and we don't need to update on the boundary layer
        if( !walk_info.at_terminus )
        {
            // Node table hasn't been updated yet, so node should be in there.
            NodeTable::Row row = db->GetNodeRow(walk_info.node); 
            ASSERT( row.incoming_xlinks.count(walk_info.xlink)==1 );
            
            // Track the xlinks we've reached for each node
            xlinks_reached_for_node[walk_info.node].insert(walk_info.xlink);        

            // Only remove if this was the last incoming XLink to the node
            if( xlinks_reached_for_node.at(walk_info.node) == row.incoming_xlinks )
                EraseSolo( category_ordering, walk_info.node );               

            // Only remove if this was the last incoming XLink to the node
            if( xlinks_reached_for_node.at(walk_info.node) == row.incoming_xlinks ) // TODO merge these ifs
                EraseSolo( simple_compare_ordering, walk_info.node );               
            //TODO refactor duplication
        } 
        
        // We must delete SimpleCompare index entries for ancestors of the base
        // node, since removing it will invalidate the SC ordering. Base is
        // sufficient: what is ancestor of base is ancestor of every node in
        // the zone. If we act at root, there won't be any.
        XLink ancestor_xlink = walk_info.xlink;
        if( walk_info.at_base ) // at base 
        {
            while( ancestor_xlink = db->TryGetParentXLink(ancestor_xlink) )
            {
                // Assume there was only one incoming XLink to the node because not a leaf
                TreePtr<Node> ancestor_node = ancestor_xlink.GetChildTreePtr();
                EraseSolo( simple_compare_ordering, ancestor_node );                              
            }
        }
        
        if( walk_info.at_base )
			xlinks_reached_for_node.clear(); // clear at end of wind-out walk
    };
}

        
DBWalk::Action Orderings::GetInsertGeometricAction()
{
    return [=](const DBWalk::WalkInfo &walk_info)
    { 
        InsertSolo( depth_first_ordering, walk_info.xlink );
        
        // Intrinsic orderings are keyed on nodes, and we don't need to update on the boundary layer
        if( !walk_info.at_terminus )
        {        
            // Only if not already
            if( category_ordering.count(walk_info.node)==0 )
                InsertSolo( category_ordering, walk_info.node );               

            // Only if not already
            if( simple_compare_ordering.count(walk_info.node)==0 )
                InsertSolo( simple_compare_ordering, walk_info.node );               
        }
    
        // We may now re-instate SimpleCompare index entries for parents 
        // of the base node so that the SC ordering is intact. Base is
        // sufficient: what is ancestor of base is ancestor of every node in
        // the zone. If we act at root, there won't be any.
        XLink ancestor_xlink = walk_info.xlink;
        if( walk_info.at_base ) // at base
        {
            while( ancestor_xlink = db->TryGetParentXLink(ancestor_xlink) )
            {
                // Assume there is only one incoming XLink to the node because not a leaf
                TreePtr<Node> ancestor_node = ancestor_xlink.GetChildTreePtr();
                InsertSolo( simple_compare_ordering, ancestor_node );                              
            }
        }
    };
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
void CheckEqualOrdering( string name, const ORDERING &l, const ORDERING &r )
{
    auto lk = KeysToSet(l);
    auto rk = KeysToSet(r);
    ASSERT(IsEquivalent(lk, rk))(name)(" ordering mismatch:\n")(DiffTrace(lk, rk))("\nReference ordering:\n")(l);
}


void Orderings::CheckEqual( shared_ptr<Orderings> l, shared_ptr<Orderings> r )
{
    CheckEqualOrdering( "DF", l->depth_first_ordering, r->depth_first_ordering );
    CheckEqualOrdering( "CAT", l->category_ordering, r->category_ordering );
    CheckEqualOrdering( "SC", l->simple_compare_ordering, r->simple_compare_ordering );
}
