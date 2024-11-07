#include "tz_overlap.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "zone_commands.hpp"
#include "tz_relation.hpp"

#include <iostream>

using namespace SR;

// ------------------------- TreeZoneOverlapHandler --------------------------

TreeZoneOverlapHandler::TreeZoneOverlapHandler( const XTreeDatabase *db_ ) :
	db( db_ )
{
}


void TreeZoneOverlapHandler::Run( shared_ptr<ZoneExpression> &root_expr )
{
	TreeZoneRelation tz_relation( db );

	// Inner and outer loops only look at PopulateTreeZoneOperator exprs
	ZoneExpression::ForDepthFirstWalk( root_expr, nullptr, [&](shared_ptr<ZoneExpression> &l_expr)
	{
		if( auto l_ptz_op = dynamic_pointer_cast<PopulateTreeZoneOperator>(l_expr) )
		{			
			// We will establish an increasing region of known non-overlapping tree zones. Detect
			// when the new l has an overlap in that zone.
			ZoneExpression::ForDepthFirstWalk( root_expr, nullptr, [&](shared_ptr<ZoneExpression> &r_expr)
			{
				if( auto r_ptz_op = dynamic_pointer_cast<PopulateTreeZoneOperator>(r_expr) )
				{			
					if( l_expr == r_expr ) // inner "r" loop stops before catching up with outer "l" loop
						LLBreak();
					
					auto p = tz_relation.CompareHierarchical( l_ptz_op->GetZone(), r_ptz_op->GetZone() );

					// Act on any overlap including equality. 
					if( p.second == ZoneRelation::OVERLAP_GENERAL || 
						p.second == ZoneRelation::OVERLAP_TERMINII ||
						p.second == ZoneRelation::EQUAL )
					{
						TRACE("CH(")(l_ptz_op->GetZone())(", ")(r_ptz_op->GetZone())(") is ")(p)("\n");
						// TODO decide which to duplicated based on size of tree zone: dup the smallest.
						// It should be possible to maintin "size of subtree" info for nodes
						if( false ) // Assume r is samller because it's to the right in the DF ordering
						{
							TRACE("Duplicate left ")(l_ptz_op)("\n");
							l_expr = l_ptz_op->DuplicateToFree(db);
							LLBreak(); // no need to check any more r for this l
						}
						else
						{
							TRACE("Duplicate right ")(r_ptz_op)("\n");
							r_expr = r_ptz_op->DuplicateToFree(db);
							// later iterations of r loop will skip over this because it's now a 
							// PopulateFreeZoneOperator, not a PopulateTreeZoneOperator
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


void TreeZoneOverlapHandler::Check( shared_ptr<ZoneExpression> &root_expr )
{
	TreeZoneRelation tz_relation( db );

	ZoneExpression::ForDepthFirstWalk( root_expr, nullptr, [&](shared_ptr<ZoneExpression> &l_expr)
	{
		if( auto l_ptz_op = dynamic_pointer_cast<PopulateTreeZoneOperator>(l_expr) )
		{			
			ZoneExpression::ForDepthFirstWalk( root_expr, nullptr, [&](shared_ptr<ZoneExpression> &r_expr)
			{
				if( auto r_ptz_op = dynamic_pointer_cast<PopulateTreeZoneOperator>(r_expr) )
				{			
					if( l_expr == r_expr ) 
						LLBreak();
					
					auto p = tz_relation.CompareHierarchical( l_ptz_op->GetZone(), r_ptz_op->GetZone() );					
					if( p.second == ZoneRelation::OVERLAP_GENERAL || 
						p.second == ZoneRelation::OVERLAP_TERMINII ||
						p.second == ZoneRelation::EQUAL )
					{
						ASSERT(false)("Tree zone overlap: ")(l_ptz_op->GetZone())(" and ")(r_ptz_op->GetZone());
					}
				}
			} );
        }
	} );	
}
