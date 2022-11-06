#include "indexes.hpp"

#include "x_tree_database.hpp"
#include "sc_relation.hpp"
#include "lacing.hpp"
#include "relation_test.hpp"

#include "common/read_args.hpp"

using namespace SR;    

//#define TRACE_CATEGORY_RELATION

Indexes::Indexes( shared_ptr<Lacing> lacing, const XTreeDatabase *db_, bool ref_ ) :
    plan( lacing ),
    depth_first_ordered_index( db_ ),
    category_ordered_index( plan.lacing ),
    db( db_ ),
    ref( ref_ ),
    use_incremental( ref ? false : ReadArgs::use_incremental )
{ 
}


Indexes::Plan::Plan( shared_ptr<Lacing> lacing_ ) :
    lacing( lacing_ )
{
}    


const Lacing *Indexes::GetLacing() const
{
    return plan.lacing.get();
}


void Indexes::PrepareDelete( DBWalk::Actions &actions )
{
	actions.indexes_out = [=](const DBWalk::WalkInfo &walk_info)
	{
		TRACE("Erasing from indexes ")(walk_info.xlink)("\n");
		EraseSolo( category_ordered_index, walk_info.xlink );       

		EraseSolo( simple_compare_ordered_index, walk_info.xlink );
        
        // We must delete SimpleCompare index entries for parents of the base
        // node, since removing it will invalidate the SC ordering.
        XLink ancestor_xlink = walk_info.xlink;
        if( walk_info.context==DBWalk::UNKNOWN ) // at base
        {
            while( ancestor_xlink = db->TryGetParentXLink(ancestor_xlink) )
            {
                EraseSolo( simple_compare_ordered_index, ancestor_xlink );
            }
        }

		EraseSolo( depth_first_ordered_index, walk_info.xlink );
	};
}


void Indexes::PrepareInsert(DBWalk::Actions &actions)
{
	actions.indexes_in = [=](const DBWalk::WalkInfo &walk_info)
	{ 
        InsertSolo( category_ordered_index, walk_info.xlink );

		InsertSolo( simple_compare_ordered_index, walk_info.xlink );		

        // We may now re-instate SimpleCompare index entries for parents 
        // of the base node so that the SC ordering is intact.
        XLink ancestor_xlink = walk_info.xlink;
        if( walk_info.context==DBWalk::UNKNOWN ) // at base
        {
            while( ancestor_xlink = db->TryGetParentXLink(ancestor_xlink) )
            {
                InsertSolo( simple_compare_ordered_index, ancestor_xlink );
            }
        }

        InsertSolo( depth_first_ordered_index, walk_info.xlink );
	};
}


void Indexes::Dump() const
{
    TRACE("category_ordered_index:\n")(category_ordered_index)("\n");
}


void Indexes::ExpectMatching( const Indexes &mut )
{
    ASSERT( ref );
    ASSERT( !mut.ref );
    
    ASSERT( depth_first_ordered_index == mut.depth_first_ordered_index )
          ( DiffTrace(depth_first_ordered_index, mut.depth_first_ordered_index) );
}


void Indexes::TestRelations( const unordered_set<XLink> &xlinks )
{
	SimpleCompareRelation scr;
	scr.Test( xlinks );

    TestOrderingIntact( simple_compare_ordered_index,
                        true,
                        "simple_compare_ordered_index" );

	CategoryRelation cat_r( plan.lacing );
	cat_r.Test( xlinks );
	
    TestOrderingIntact( category_ordered_index,
                        true,
                        "category_ordered_index" );

	DepthFirstRelation dfr( db );
	dfr.Test( xlinks );	

    TestOrderingIntact( depth_first_ordered_index,
                        true,
                        "depth_first_ordered_index" );
}


