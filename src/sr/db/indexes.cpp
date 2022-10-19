#include "indexes.hpp"

#include "x_tree_database.hpp"
#include "sc_relation.hpp"
#include "link_table.hpp"
#include "lacing.hpp"

#include "common/read_args.hpp"

using namespace SR;    

//#define TRACE_CATEGORY_RELATION


Indexes::Indexes( shared_ptr<Lacing> lacing, const LinkTable *link_table_, bool ref_ ) :
    plan( lacing ),
    depth_first_ordered_index( link_table_ ),
    category_ordered_index( plan.lacing ),
    link_table( link_table_ ),
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


void Indexes::MonolithicClear()
{
    depth_first_ordered_index.clear();
}


void Indexes::PrepareMonolithicBuild(DBWalk::Actions &actions)
{
	actions.indexes_in_late = [&](const DBWalk::WalkInfo &walk_info)
	{
		depth_first_ordered_index.insert( walk_info.xlink );
	};
}


void Indexes::PrepareDelete( DBWalk::Actions &actions )
{
	actions.indexes_in = [&](const DBWalk::WalkInfo &walk_info)
	{
		EraseSolo( category_ordered_index, walk_info.xlink );       
		EraseSolo( simple_compare_ordered_index, walk_info.xlink );
	};
}


void Indexes::PrepareInsert(DBWalk::Actions &actions)
{
	actions.indexes_in_late = [&](const DBWalk::WalkInfo &walk_info)
	{ 
        category_ordered_index.insert( walk_info.xlink );
		simple_compare_ordered_index.insert( walk_info.xlink );		
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
    
    ASSERT( category_ordered_index == mut.category_ordered_index )
          ( DiffTrace(category_ordered_index, mut.category_ordered_index) )
          ( mut.category_ordered_index );
}


void Indexes::TestRelations( const unordered_set<XLink> &xlinks )
{
	SimpleCompareRelation scr;
	scr.Test( xlinks );

	CategoryRelation cat_r( plan.lacing );
	cat_r.Test( xlinks );
	
	DepthFirstRelation dfr( link_table );
	dfr.Test( xlinks );	
}


