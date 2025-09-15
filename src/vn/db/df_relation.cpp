#include "df_relation.hpp"

#include "relation_test.hpp"
#include "x_tree_database.hpp"

#include "helpers/simple_compare.hpp"
#include "helpers/simple_duplicate.hpp"

#include <random>

using namespace VN;

DepthFirstRelation::DepthFirstRelation(const XTreeDatabase *db_) :
    db( db_ )
{
}


bool DepthFirstRelation::operator()( KeyType l_key, KeyType r_key ) const
{
    return Compare3Way(l_key, r_key) < 0;
}


Orderable::Diff DepthFirstRelation::Compare3Way( KeyType l_key, KeyType r_key ) const
{
    return CompareHierarchical( l_key, r_key ).first;
}


pair<Orderable::Diff, DepthFirstRelation::RelType> DepthFirstRelation::CompareHierarchical( KeyType l_key, KeyType r_key ) const
{
    // Maps a parent xlink to two optional child xlinks: the first is 
    // a weak ancestor of l and second of r. We fill in first from the
    // ancestry of l and the second from r. As soon as we discover we've 
    // filled both child xlinks in for the same parent, we're done.
    map<XLink, pair<XLink, XLink>> candidate_mutuals;

    //FTRACE("\nComparing ")(l_key)(" with ")(r_key)("\n");
    
    if( l_key == r_key )
    {
        //FTRACE("early out\n");
        return make_pair(0, EQUAL);
    }
    
    // Parent is lower in depth-first ordering
    
    XLink l_cur_xlink = l_key;
    XLink r_cur_xlink = r_key;
    while(true)
    {
        XLink l_parent_xlink = db->TryGetParentXLink(l_cur_xlink);    
        XLink r_parent_xlink = db->TryGetParentXLink(r_cur_xlink);
        
        //FTRACE("At ")(l_cur_xlink)(" and ")(r_cur_xlink)("\n")
        //      ("Parents ")(l_parent_xlink)(" and ")(r_parent_xlink)("\n")
        //      ("With ")(candidate_mutuals)("\n");

        if( !l_parent_xlink && !r_parent_xlink )
        {
            //FTRACE("Both at root, comparing root ordinals\n");
            const LinkTable::Row &l_row = db->GetRow(l_cur_xlink);       
            const LinkTable::Row &r_row = db->GetRow(r_cur_xlink);
            if( l_row.tree_ordinal == r_row.tree_ordinal )
				db->DumpTables();
            ASSERT( l_row.tree_ordinal != r_row.tree_ordinal )
                  ("At ")(l_cur_xlink)(" and ")(r_cur_xlink)("\n")
                  ("Parents ")(l_parent_xlink)(" and ")(r_parent_xlink)("\n")
                  ("Ordinals %u and %u\n", l_row.tree_ordinal, r_row.tree_ordinal)
                  ("Candidate mutuals ")(candidate_mutuals)("\n");
            return make_pair(static_cast<int>(l_row.tree_ordinal) - static_cast<int>(r_row.tree_ordinal), ROOT_SIBLINGS);
        }

        if( l_parent_xlink )
        {            
            // If l hits r0 then l0 was a descendent of it. Use parent to spot sooner.
            if( l_parent_xlink == r_key )
                return make_pair(1, RIGHT_IS_ANCESTOR);
                
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
            l_cur_xlink = l_parent_xlink; // advance l toward ancestor
        }

        if( r_parent_xlink )
        {
            // If r hits l0 then r0 was a descendent of it. Use parent to spot sooner.
            if( r_parent_xlink == l_key )
                return make_pair(-1, LEFT_IS_ANCESTOR);

            // If we share a parent, l0 and r0 are weakly removed siblings
            if( candidate_mutuals[r_parent_xlink].first )
            {
                // Ensure l and r are direct siblings
                l_cur_xlink = candidate_mutuals[r_parent_xlink].first;
                //FTRACE("Paths met advancing right, parent is: ")(r_parent_xlink)("\n");
                break;
            }
            // Update candidate_mutuals.
            candidate_mutuals[r_parent_xlink].second = r_cur_xlink;
            r_cur_xlink = r_parent_xlink; // advance r toward ancestor
        }
        
        ASSERT( l_cur_xlink != r_cur_xlink );
    }
    
    // Dropping out of loop means l0 and r0 are weakly removed siblings
    // and l and r are direct siblings. We must check against orderings in 
    // the common parent to l and r and can use link table rows directly 
    // for this due to having a common parent.
    const LinkTable::Row &l_row = db->GetRow(l_cur_xlink);       
    const LinkTable::Row &r_row = db->GetRow(r_cur_xlink);
    
    // Itemisation is primary
    if( Orderable::Diff d = l_row.item_ordinal - r_row.item_ordinal )
        return make_pair(d, ITEM_SIBLINGS);
        
    // Secondary is position inside container
    if( Orderable::Diff d = l_row.container_ordinal - r_row.container_ordinal )
        return make_pair(d, CONTAINER_SIBLINGS);
        
    ASSERT(false)
          ("Comparing ")(l_key)(" with ")(r_key)("\n")
          ("Got to ")(l_cur_xlink)(" and ")(r_cur_xlink)("\n")
          (candidate_mutuals);
    ASSERTFAIL();
}


void DepthFirstRelation::Test( const vector<KeyType> &keys )
{
    using namespace std::placeholders;

    TestRelationProperties<KeyType>( keys,
                                     true,
                                     "DepthFirstRelation",
                                     bind(&DepthFirstRelation::Compare3Way, *this, _1, _2), 
    [&](KeyType l, KeyType r) -> bool
    { 
        return l==r; 
    } );
}

