#include "fz_merge.hpp"

#include "zone_expressions.hpp"
#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tz_relation.hpp"

#include <iostream>

using namespace SR;

// ------------------------- FreeZoneMerger --------------------------

FreeZoneMerger::FreeZoneMerger()
{
}
	

void FreeZoneMerger::Run( shared_ptr<Layout> &root_expr )
{
	Layout::ForDepthFirstWalk( root_expr, nullptr, [&](shared_ptr<Layout> &expr)
	{
		if( auto pfz_op = dynamic_pointer_cast<MergeFreeZoneOperator>(expr) )
        {
			TRACE("Parent MergeFreeZoneOperator ")(*pfz_op)("\n");
			FreeZone &free_zone = pfz_op->GetZone();
			ASSERT( !free_zone.IsEmpty() );

			FreeZone::TerminusIterator it_t = free_zone.GetTerminiiBegin();
			MergeFreeZoneOperator::ChildExpressionIterator it_child = pfz_op->GetChildrenBegin();
			
			while( it_child != pfz_op->GetChildrenEnd() )
			{
				ASSERT( it_t != free_zone.GetTerminiiEnd() ); // length mismatch		
				if( auto child_pfz_op = dynamic_pointer_cast<MergeFreeZoneOperator>(*it_child) )
				{	
					TRACE("Child MergeFreeZoneOperator ")(*child_pfz_op)(" and terminus ")(*it_t)("\n");
					FreeZone &child_free_zone = child_pfz_op->GetZone();
					it_t = free_zone.MergeTerminus( it_t, make_unique<FreeZone>(child_free_zone) );		
					TRACE("Mutator OK\n");
					it_child = pfz_op->SpliceOver( it_child, child_pfz_op->MoveChildExpressions() );
					TRACE("Splice OK\n");
				}	
				else
				{
					TRACE("Child DupMergeTreeZoneOperator: SKIPPING and terminus ")(*it_t)("\n");
					it_t++;
					it_child++;
				}						
			} 
			ASSERT( it_t == free_zone.GetTerminiiEnd() ); // length mismatch	
			TRACE("Loop OK\n");
		}
	} );			
}


void FreeZoneMerger::Check( shared_ptr<Layout> &root_expr )
{
	Layout::ForDepthFirstWalk( root_expr, nullptr, [&](shared_ptr<Layout> &expr)
	{
		if( auto pz_op = dynamic_pointer_cast<MergeFreeZoneOperator>(expr) )
        {
			pz_op->ForChildren([&](shared_ptr<Layout> &child_expr)
			{
				if( auto child_pz_op = dynamic_pointer_cast<MergeFreeZoneOperator>(child_expr) )
					ASSERT(false)("Free zone ")(*expr)(" touching another free zone ")(*child_expr);
			} );
		}
	} );		
}
