#include "df_relation.hpp"

#include "relation_test.hpp"
#include "link_table.hpp"

#include "helpers/simple_compare.hpp"
#include "helpers/duplicate.hpp"

#include <random>

#define NEW_DF_REL

using namespace SR;

DepthFirstRelation::DepthFirstRelation(const LinkTable *link_table_) :
    link_table( link_table_ )
{
}


bool DepthFirstRelation::operator()( XLink l_xlink, XLink r_xlink ) const
{
    return Compare3Way(l_xlink, r_xlink) < 0;
}


Orderable::Diff DepthFirstRelation::Compare3Way( const XLink l_xlink, const XLink r_xlink ) const
{
#ifdef NEW_DF_REL
    // Maps a parent xlink to two optional child xlinks: the first is 
    // a weak ancestor of l and second of r. We fill in first from the
    // ancestry of l and the second from r. As soon as we discover we've 
    // filled both child xlinks in for the same parent, we're done.
    map<XLink, pair<XLink, XLink>> candidate_mutuals;

    //FTRACE("\nComparing ")(l_xlink)(" with ")(r_xlink)("\n");
    
    if( l_xlink == r_xlink )
    {
        //FTRACE("early out\n");
        return 0;
    }
    
    // Parent is lower in depth-first ordering
    const Orderable::Diff LEFT_IS_ANCESTOR  = 0-1;
    const Orderable::Diff RIGHT_IS_ANCESTOR = 1-0;
    
    XLink l_cur_xlink = l_xlink;
    XLink r_cur_xlink = r_xlink;
    while(true)
    {
        const LinkTable::Row &l_row = link_table->GetRow(l_cur_xlink);  
        XLink l_parent_xlink = l_row.parent_xlink;    
        const LinkTable::Row &r_row = link_table->GetRow(r_cur_xlink);
        XLink r_parent_xlink = r_row.parent_xlink;
        
        //FTRACE("At ")(l_cur_xlink)(" and ")(r_cur_xlink)("\n")
        //      ("Parents ")(l_parent_xlink)(" and ")(r_parent_xlink)("\n")
        //      ("With ")(candidate_mutuals)("\n");

        if( l_row.IsBase() && r_row.IsBase() )
        {
            //FTRACE("Both at base, comparing base ordinals\n");
            return l_row.base_ordinal - r_row.base_ordinal;
        }
        else
        {
            if( !l_row.IsBase() )
            {            
                // If we hit r0 then l0 was a descendent of it. Use parent to spot sooner.
                if( l_parent_xlink == r_xlink )
                    return RIGHT_IS_ANCESTOR;
                    
                // If we share a parent, l0 and r0 are weakly removed siblings
                if( candidate_mutuals[l_parent_xlink].second )
                {
                    // Ensure l and r are direct siblings
                    r_cur_xlink = candidate_mutuals[l_parent_xlink].second;
                    //FTRACE("Paths met advancing left, parent is: ")(l_parent_xlink)("\n");
                    break;
                }
                // If l and r are same depth, we need to update candidate_mutuals here
                // so the R block just below can spot the meet.
                candidate_mutuals[l_parent_xlink].first = l_cur_xlink;                
            }

            if( !r_row.IsBase() )
            {
                // If we hit l0 then r0 was a descendent of it. Use parent to spot sooner.
                if( r_parent_xlink == l_xlink )
                    return LEFT_IS_ANCESTOR;

                // If we share a parent, l0 and r0 are weakly removed siblings
                if( candidate_mutuals[r_parent_xlink].first )
                {
                    // Ensure l and r are direct siblings
                    l_cur_xlink = candidate_mutuals[r_parent_xlink].first;
                    //FTRACE("Paths met advancing right, parent is: ")(r_parent_xlink)("\n");
                    break;
                }
                // Update candidate mutual
                candidate_mutuals[r_parent_xlink].second = r_cur_xlink;
            }

            if( !l_row.IsBase() )
            {
                l_cur_xlink = l_parent_xlink;
            }
            if( !r_row.IsBase() )
            {
                r_cur_xlink = r_parent_xlink;
            }
        }
    }
    
    // Dropping out of loop means l0 and r0 are weakly removed siblings
    // and l and r are direct siblings. We must check against orderings in 
    // the common parent to l and r and can use link table rows directly 
    // for this due to having a common parent.
    
    const LinkTable::Row &l_row = link_table->GetRow(l_cur_xlink);       
    const LinkTable::Row &r_row = link_table->GetRow(r_cur_xlink);
    if( Orderable::Diff d = l_row.item_number - r_row.item_number )
        return d;
        
    if( Orderable::Diff d = l_row.container_ordinal - r_row.container_ordinal )
        return d;
        
    ASSERT(false)
          ("Comparing ")(l_xlink)(" with ")(r_xlink)("\n")
          ("Got to ")(l_cur_xlink)(" and ")(r_cur_xlink)("\n")
          (candidate_mutuals);
    return 0;
#else
	const LinkTable::Row &l_row = link_table->GetRow(l_xlink);
	const LinkTable::Row &r_row = link_table->GetRow(r_xlink);

    return l_row.depth_first_ordinal - r_row.depth_first_ordinal;
#endif
}


void DepthFirstRelation::Test( const unordered_set<XLink> &xlinks )
{
	using namespace std::placeholders;

	TestRelationProperties( xlinks,
                            true,
                            "DepthFirstRelation",
                            [&](){ return Trace(link_table); },                            
                            bind(&DepthFirstRelation::Compare3Way, *this, _1, _2), 
    [&](XLink l, XLink r)
    { 
        return l==r; 
    } );
}

