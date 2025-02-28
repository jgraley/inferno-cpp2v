#include "tz_overlap.hpp"

#include "patches.hpp"
#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tz_relation.hpp"

#include <iostream>

using namespace SR;

// ------------------------- TreeZoneOverlapHandler --------------------------

TreeZoneOverlapHandler::TreeZoneOverlapHandler( const XTreeDatabase *db_ ) :
	db( db_ )
{
}


void TreeZoneOverlapHandler::Run( shared_ptr<Patch> &layout )
{
	TreeZoneRelation tz_relation( db );

	// Inner and outer loops only look at TreeZonePatch patches
	Patch::ForDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &l_patch)
	{
		if( auto left_tree_patch = dynamic_pointer_cast<TreeZonePatch>(l_patch) )
		{			
			// We will establish an increasing region of known non-overlapping tree zones. Detect
			// when the new l has an overlap in that zone.
			Patch::ForDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &r_patch)
			{
				if( auto right_tree_patch = dynamic_pointer_cast<TreeZonePatch>(r_patch) )
				{			
					if( l_patch == r_patch ) // inner "r" loop stops before catching up with outer "l" loop
						LLBreak();
					
					auto p = tz_relation.CompareHierarchical( left_tree_patch->GetZone(), right_tree_patch->GetZone() );

					// Act on any overlap including equality. 
					if( p.second == ZoneRelation::OVERLAP_GENERAL || 
						p.second == ZoneRelation::OVERLAP_TERMINII ||
						p.second == ZoneRelation::EQUAL )
					{
						TRACE("CH(")(left_tree_patch->GetZone())(", ")(right_tree_patch->GetZone())(") is ")(p)("\n");
						// TODO decide which to duplicated based on size of tree zone: dup the smallest.
						// It should be possible to maintin "size of subtree" info for nodes
						if( false ) // Assume r is samller because it's to the right in the DF ordering
						{
							TRACE("Duplicate left ")(left_tree_patch)("\n");
							l_patch = left_tree_patch->DuplicateToFree();
							LLBreak(); // no need to check any more r for this l
						}
						else
						{
							TRACE("Duplicate right ")(right_tree_patch)("\n");
							r_patch = right_tree_patch->DuplicateToFree();
							// later iterations of r loop will skip over this because it's now a 
							// FreeZonePatch, not a TreeZonePatch
						}
												
						// TODO duplicate r here rather than l later: we'd prefer to duplicate r
						// because it's more likely to be deeper or leaf. Future algos would recurse
						// explicitly and make sure the one that gets duplciated is deeper.
					}
				}
			} );
        }
	} );	
}


void TreeZoneOverlapHandler::Check( shared_ptr<Patch> &layout )
{
	TreeZoneRelation tz_relation( db );

	Patch::ForDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &l_patch)
	{
		if( auto left_tree_patch = dynamic_pointer_cast<TreeZonePatch>(l_patch) )
		{			
			Patch::ForDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &r_patch)
			{
				if( auto right_tree_patch = dynamic_pointer_cast<TreeZonePatch>(r_patch) )
				{			
					if( l_patch == r_patch ) 
						LLBreak();
					
					auto p = tz_relation.CompareHierarchical( left_tree_patch->GetZone(), right_tree_patch->GetZone() );					
					if( p.second == ZoneRelation::OVERLAP_GENERAL || 
						p.second == ZoneRelation::OVERLAP_TERMINII ||
						p.second == ZoneRelation::EQUAL )
					{
						ASSERT(false)("Tree zone overlap: ")(left_tree_patch->GetZone())(" and ")(right_tree_patch->GetZone());
					}
				}
			} );
        }
	} );	
}
