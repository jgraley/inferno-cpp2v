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


DBWalk::Action Orderings::GetDeleteAction()
{	
	return [=](const DBWalk::WalkInfo &walk_info)
	{
		TRACE("Erasing from orderings ")(walk_info.xlink)("\n");
		EraseSolo( category_ordering, walk_info.xlink );       

		EraseSolo( simple_compare_ordering, walk_info.xlink );
        
        // We must delete SimpleCompare index entries for ancestors of the base
        // node, since removing it will invalidate the SC ordering. Base is
        // sufficient: what is ancestor of base is ancestor of every node in
        // the zone. If we act at root, there won't be any.
        XLink ancestor_xlink = walk_info.xlink;
        if( walk_info.base ) // at base 
        {
            while( ancestor_xlink = db->TryGetParentXLink(ancestor_xlink) )
            {
                EraseSolo( simple_compare_ordering, ancestor_xlink );
            }
        }

		EraseSolo( depth_first_ordering, walk_info.xlink );
	};
}

		
DBWalk::Action Orderings::GetInsertAction()
{
	return [=](const DBWalk::WalkInfo &walk_info)
	{ 
        InsertSolo( category_ordering, walk_info.xlink );

		InsertSolo( simple_compare_ordering, walk_info.xlink );		

        // We may now re-instate SimpleCompare index entries for parents 
        // of the base node so that the SC ordering is intact. Base is
        // sufficient: what is ancestor of base is ancestor of every node in
        // the zone. If we act at root, there won't be any.
        XLink ancestor_xlink = walk_info.xlink;
        if( walk_info.base ) // at base
        {
            while( ancestor_xlink = db->TryGetParentXLink(ancestor_xlink) )
            {
                InsertSolo( simple_compare_ordering, ancestor_xlink );
            }
        }

        InsertSolo( depth_first_ordering, walk_info.xlink );
	};
}


void Orderings::Dump() const
{
    TRACE("category_ordering:\n")(category_ordering)("\n");
}


void Orderings::TestRelations( const unordered_set<XLink> &xlinks )
{
	SimpleCompareRelation scr;
	scr.Test( xlinks );

    TestOrderingIntact( simple_compare_ordering,
                        true,
                        "simple_compare_ordering" );

	CategoryRelation cat_r( plan.lacing );
	cat_r.Test( xlinks );
	
    TestOrderingIntact( category_ordering,
                        true,
                        "category_ordering" );

	DepthFirstRelation dfr( db );
	dfr.Test( xlinks );	

    TestOrderingIntact( depth_first_ordering,
                        true,
                        "depth_first_ordering" );
}


