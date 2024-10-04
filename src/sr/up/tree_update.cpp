#include "tree_update.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "commands.hpp"
#include "tz_relation.hpp"

#include <iostream>

using namespace SR;

// ------------------------- Runners --------------------------

FreeZone SR::RunForBuilder( const FreeZoneExpression *expr )
{
	unique_ptr<FreeZone> free_zone = expr->Evaluate();   
	return *free_zone;
}


void SR::RunForReplace( const Command *initial_cmd, XTreeDatabase *x_tree_db )
{
	shared_ptr<FreeZoneExpression> expr = dynamic_cast<const UpdateTreeCommand &>(*initial_cmd).GetExpression();

	// TODO automated enactment of free zone markers (Make them be lambdas first and don't 
	// pass scr_engine into PopulateX expressions!). Marker -> OnLocatedAction
	
	EmptyZoneElider().Run(expr);
	EmptyZoneElider().Check(expr);
	
	TreeZoneOverlapHandler( x_tree_db ).Run(expr);
	TreeZoneOverlapHandler( x_tree_db ).Check(expr);
	
	TreeZoneOrderingHandler( x_tree_db ).Run(expr);
	TreeZoneOrderingHandler( x_tree_db ).Check(expr);
	
	// TODO deep check for overlaps and ordering

	FreeZoneMerger().Run(expr);  // TODO fix me!!
	FreeZoneMerger().Check(expr);
	
	// TODO reductive inversion using Quark algo
	
	// TODO enact tree zone markers (here or in DB)

	// TODO merge tree zones and check initial update command is now trivial
	
	// Execute it
	initial_cmd->Execute();   
}

// ------------------------- TreeZoneOverlapHandler --------------------------

TreeZoneOverlapHandler::TreeZoneOverlapHandler( const XTreeDatabase *db_ ) :
	db( db_ )
{
}


void TreeZoneOverlapHandler::Run( shared_ptr<FreeZoneExpression> &base )
{
	TreeZoneRelation tz_relation( db );

	// Inner and outer loops only look at PopulateTreeZoneOperator exprs
	FreeZoneExpression::ForDepthFirstWalk( base, nullptr, [&](shared_ptr<FreeZoneExpression> &l_expr)
	{
		if( auto l_ptz_op = dynamic_pointer_cast<PopulateTreeZoneOperator>(l_expr) )
		{			
			// We will establish an increasing region of known non-overlapping tree zones. Detect
			// when the new l has an overlap in that zone.
			FreeZoneExpression::ForDepthFirstWalk( base, nullptr, [&](shared_ptr<FreeZoneExpression> &r_expr)
			{
				if( auto r_ptz_op = dynamic_pointer_cast<PopulateTreeZoneOperator>(r_expr) )
				{			
					if( l_expr == r_expr ) // inner "r" loop stops before catching up with outer "l" loop
						Break();
					
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
							l_expr = l_ptz_op->DuplicateToFree();
							Break(); // no need to check any more r for this l
						}
						else
						{
							TRACE("Duplicate right ")(r_ptz_op)("\n");
							r_expr = r_ptz_op->DuplicateToFree();
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


void TreeZoneOverlapHandler::Check( shared_ptr<FreeZoneExpression> &base )
{
	TreeZoneRelation tz_relation( db );

	FreeZoneExpression::ForDepthFirstWalk( base, nullptr, [&](shared_ptr<FreeZoneExpression> &l_expr)
	{
		if( auto l_ptz_op = dynamic_pointer_cast<PopulateTreeZoneOperator>(l_expr) )
		{			
			FreeZoneExpression::ForDepthFirstWalk( base, nullptr, [&](shared_ptr<FreeZoneExpression> &r_expr)
			{
				if( auto r_ptz_op = dynamic_pointer_cast<PopulateTreeZoneOperator>(r_expr) )
				{			
					if( l_expr == r_expr ) 
						Break();
					
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

// ------------------------- EmptyZoneElider --------------------------

EmptyZoneElider::EmptyZoneElider()
{
}
	

void EmptyZoneElider::Run( shared_ptr<FreeZoneExpression> &base )
{
	FreeZoneExpression::ForDepthFirstWalk( base, nullptr, [&](shared_ptr<FreeZoneExpression> &expr)
	{
		if( auto pz_op = dynamic_pointer_cast<PopulateZoneOperator>(expr) )
            if( pz_op->GetZone().IsEmpty() )
            {
				shared_ptr<FreeZoneExpression> child_expr = OnlyElementOf( pz_op->GetChildExpressions() );
				if( auto child_pz_op = dynamic_pointer_cast<PopulateZoneOperator>(child_expr) )
					child_pz_op->AddEmbeddedMarkers( pz_op->GetEmbeddedMarkers() );
				expr = child_expr;
			}
	} );	
}


void EmptyZoneElider::Check( shared_ptr<FreeZoneExpression> &base )
{
	FreeZoneExpression::ForDepthFirstWalk( base, nullptr, [&](shared_ptr<FreeZoneExpression> &expr)
	{
		if( auto pz_op = dynamic_pointer_cast<PopulateZoneOperator>(expr) )
            ASSERT( !pz_op->GetZone().IsEmpty() )("Found empty zone in populate op: ")(pz_op->GetZone());
	} );	
}

// ------------------------- FreeZoneMerger --------------------------

FreeZoneMerger::FreeZoneMerger()
{
}
	

void FreeZoneMerger::Run( shared_ptr<FreeZoneExpression> &base )
{
	FreeZoneExpression::ForDepthFirstWalk( base, nullptr, [&](shared_ptr<FreeZoneExpression> &expr)
	{
		if( auto pfz_op = dynamic_pointer_cast<PopulateFreeZoneOperator>(expr) )
        {
			TRACE("Parent PopulateFreeZoneOperator ")(*pfz_op)("\n");
			FreeZone &free_zone = pfz_op->GetZone();
			ASSERT( !free_zone.IsEmpty() );

			FreeZone::TerminusIterator it_t = free_zone.GetTerminiiBegin();
			PopulateFreeZoneOperator::ChildExpressionIterator it_child = pfz_op->GetChildrenBegin();
			
			while( it_child != pfz_op->GetChildrenEnd() )
			{
				ASSERT( it_t != free_zone.GetTerminiiEnd() ); // length mismatch		
				if( auto child_pfz_op = dynamic_pointer_cast<PopulateFreeZoneOperator>(*it_child) )
				{	
					TRACE("Child PopulateFreeZoneOperator ")(*child_pfz_op)(" and terminus ")(*it_t)("\n");
					FreeZone &child_free_zone = child_pfz_op->GetZone();
					it_t = free_zone.PopulateTerminus( it_t, make_unique<FreeZone>(child_free_zone) );		
					TRACE("Terminus OK\n");
					it_child = pfz_op->SpliceOver( it_child, child_pfz_op->MoveChildExpressions() );
					TRACE("Splice OK\n");
				}	
				else
				{
					TRACE("Child PopulateTreeZoneOperator: SKIPPING and terminus ")(*it_t)("\n");
					it_t++;
					it_child++;
				}						
			} 
			ASSERT( it_t == free_zone.GetTerminiiEnd() ); // length mismatch	
			TRACE("Loop OK\n");
		}
	} );			
}


void FreeZoneMerger::Check( shared_ptr<FreeZoneExpression> &base )
{
	FreeZoneExpression::ForDepthFirstWalk( base, nullptr, [&](shared_ptr<FreeZoneExpression> &expr)
	{
		if( auto pz_op = dynamic_pointer_cast<PopulateFreeZoneOperator>(expr) )
        {
			pz_op->ForChildren([&](shared_ptr<FreeZoneExpression> &child_expr)
			{
				if( auto child_pz_op = dynamic_pointer_cast<PopulateFreeZoneOperator>(child_expr) )
					ASSERT(false)("Free zone ")(*expr)(" touching another free zone ")(*child_expr);
			} );
		}
	} );		
}

// ------------------------- TreeZoneOrderingHandler --------------------------


TreeZoneOrderingHandler::TreeZoneOrderingHandler(const XTreeDatabase *db_) :
	db( db_ ),
	dfr( db )
{
}
	

void TreeZoneOrderingHandler::Run( shared_ptr<FreeZoneExpression> &base )
{
	XLink root = db->GetRootXLink();
	XLink last = db->GetLastDescendant(root);
	RunWorker( base, root, last, false );
}


void TreeZoneOrderingHandler::Check( shared_ptr<FreeZoneExpression> &base )
{
	XLink root = db->GetRootXLink();
	XLink last = db->GetLastDescendant(root);
	RunWorker( base, root, last, true );
}


void TreeZoneOrderingHandler::RunWorker( shared_ptr<FreeZoneExpression> &base, 
										 XLink range_begin,
										 XLink range_end,
										 bool just_check )
{
	TRACE("RunWorker() at ")(base)(" with range ")(range_begin)(" to ")(range_end)("\n");
	// Actions to take when we have a range. Use at root and for
	// terminii of tree zones.
	list<shared_ptr<FreeZoneExpression> *> expr_list;
	GatherTreeZoneOps( base, expr_list );
	
	for( shared_ptr<FreeZoneExpression> *expr : expr_list )
	{
		auto ptz_op = dynamic_pointer_cast<PopulateTreeZoneOperator>(*expr);
		ASSERT( ptz_op ); // should succeed due GatherTreeZoneOps()
		XLink tz_base = ptz_op->GetZone().GetBaseXLink();
		
		// Check in range supplied to us for root or parent TZ terminus
		Orderable::Diff diff_begin = dfr.Compare3Way(tz_base, range_begin);
		Orderable::Diff diff_end = dfr.Compare3Way(tz_base, range_end);
		bool ok = diff_begin >= 0 && diff_end <= 0; // both inclusive
		if( !ok )
		{     
			if( just_check )
			{
				FTRACE(db->GetOrderings().depth_first_ordering)("\n");
				ASSERT(diff_begin >= 0)("Tree zone base ")(tz_base)(" appears before limit ")(range_begin)(" in X tree");
				ASSERT(diff_end <= 0)("Tree zone base ")(tz_base)(" appears after limit ")(range_end)(" in X tree");
			}
			else
			{
				// Action: minimal is to duplicate ptz_op->GetZone() and
				// every TZ under it into free zones. Could be improved later 
				TRACE("Duplicating ")(tz_base)("\n");
				DuplicateTreeZone( *expr );
				continue; // no range threshold update or recurse
			}
		}
		TRACE(tz_base)(" OK\n");
		
	    // Narrow the acceptable range for the next tree zone
		range_begin = tz_base;

		// Recurse to check descendents of the tree zone
		RunForTreeZone( ptz_op, false );		
	}
}


void TreeZoneOrderingHandler::RunForTreeZone( shared_ptr<PopulateTreeZoneOperator> &ptz_op, 
                                              bool just_check )
{
	TRACE("RunForTreeZone() looking for free zones under ")(ptz_op)("\n");
	
	// We have a tree zone. For each of its terminii, find the acceptable
	// range of descendent tree zones and recurse.
	TreeZone tree_zone = ptz_op->GetZone();
	TreeZone::TerminusIterator it_t = tree_zone.GetTerminiiBegin();
	ptz_op->ForChildren([&](shared_ptr<FreeZoneExpression> &child_expr)	
	{
		XLink range_begin = *it_t; // inclusive (terminus XLink equals base XLink of attached tree zone)
		XLink range_end = db->GetLastDescendant(range_begin); // inclusive (is same or child of range_begin)
		RunWorker( child_expr, range_begin, range_end, just_check );
	} );
}
                                       

void TreeZoneOrderingHandler::GatherTreeZoneOps( shared_ptr<FreeZoneExpression> &expr, 
              				  		             list<shared_ptr<FreeZoneExpression> *> &tree_zones )
{
	// Get descendant tree zones, skipping over free zones, into a list for
	// convenience.
	if( auto ptz_op = dynamic_pointer_cast<PopulateTreeZoneOperator>(expr) )
	{
		tree_zones.push_back( &expr );
	}
	else if( auto pz_op = dynamic_pointer_cast<PopulateZoneOperator>(expr) )
	{
		pz_op->ForChildren( [&](shared_ptr<FreeZoneExpression> &child_expr)
		{
			GatherTreeZoneOps( child_expr, tree_zones );
		} );
	}
}
              						  
              						  
void TreeZoneOrderingHandler::DuplicateTreeZone( shared_ptr<FreeZoneExpression> &expr )
{
	// At present, for simplest algo, we DO recurse the entire subtree and 
	// duplicate everything. But it shouldn't be too hard to just duplicate the
	// supplied one and let checks continue on the children of the bad TZ. Would
	// require RunWorker() to be stated-out so it can be invoked from a walk though.
	FreeZoneExpression::ForDepthFirstWalk( expr, nullptr, [&](shared_ptr<FreeZoneExpression> &child_expr)
	{
		if( auto ptz_op = dynamic_pointer_cast<PopulateTreeZoneOperator>(child_expr) )
		{		
			TRACE("Duplicate ")(ptz_op)("\n");
			child_expr = ptz_op->DuplicateToFree();
		}
	} );	
}
