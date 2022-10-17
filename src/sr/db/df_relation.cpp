#include "df_relation.hpp"

#include "relation_test.hpp"
#include "link_table.hpp"

#include "helpers/simple_compare.hpp"
#include "helpers/duplicate.hpp"

#include <random>

//#define NEW_DF_REL

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

    FTRACE("Comparing ")(l_xlink)(" with ")(r_xlink)("\n");
    
    if( l_xlink == r_xlink )
    {
        FTRACE("early out\n");
        return 0;
    }
    
    XLink l_cur_xlink = l_xlink;
    XLink r_cur_xlink = r_xlink;
    while(true)
    {
        FTRACE("At ")(l_cur_xlink)(" and ")(r_cur_xlink)("\n")
              ("With ")(candidate_mutuals)("\n");

        const LinkTable::Row &l_row = link_table->GetRow(l_cur_xlink);       
        const LinkTable::Row &r_row = link_table->GetRow(r_cur_xlink);
        if( !l_row.IsBase() )
        {            
            XLink parent_xlink = l_row.parent_xlink;
            candidate_mutuals[parent_xlink].first = l_cur_xlink;
            if( candidate_mutuals[parent_xlink].second == l_cur_xlink )
            {
                return 1; // L is descendent of R (because R got there first)
            }
            if( candidate_mutuals[parent_xlink].second )
            {
                r_cur_xlink = candidate_mutuals[parent_xlink].second;
                break;
            }
            l_cur_xlink = parent_xlink;
        }


        if( !r_row.IsBase() )
        {
            XLink parent_xlink = r_row.parent_xlink;
            candidate_mutuals[parent_xlink].second = r_cur_xlink;
            if( candidate_mutuals[parent_xlink].first == r_cur_xlink )
            {
                return -1; // R is descendent of L (because L got there first)
            }
            if( candidate_mutuals[parent_xlink].first )
            {
                l_cur_xlink = candidate_mutuals[parent_xlink].first;
                break;
            }
            r_cur_xlink = parent_xlink;
        }
        
        if( l_row.IsBase() && r_row.IsBase() )
        {
            FTRACE("Both at base 2\n");
            return l_row.base_ordinal - r_row.base_ordinal;
        }
    }
    
    const LinkTable::Row &l_row = link_table->GetRow(l_cur_xlink);       
    const LinkTable::Row &r_row = link_table->GetRow(r_cur_xlink);
    if( Orderable::Diff d = l_row.item_number - r_row.item_number )
        return d;
        
    if( Orderable::Diff d = l_row.container_ordinal - r_row.container_ordinal )
        return d;
        
    ASSERT(false)
          ("Comparing ")(l_xlink)(" with ")(r_xlink)("\n")
          ("Found parent with children ")(l_cur_xlink)(" and ")(r_cur_xlink)("\n")
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

