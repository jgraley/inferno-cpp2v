#include "zone_expressions.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tree_update.hpp"

using namespace SR;

//#define RECURSIVE_TRACE_OPERATOR

// ------------------------- Layout --------------------------

void Layout::ForDepthFirstWalk( shared_ptr<Layout> &base,
											function<void(shared_ptr<Layout> &expr)> func_in,
	                                        function<void(shared_ptr<Layout> &expr)> func_out ) try
{
	if( func_in )
		func_in(base);
	base->DepthFirstWalkImpl(func_in, func_out);
	if( func_out )
		func_out(base);
}
catch( BreakException )
{
}		       

// ------------------------- LayoutOperator --------------------------

LayoutOperator::LayoutOperator( list<shared_ptr<Layout>> &&child_expressions_ ) :
	child_expressions(move(child_expressions_))
{
}	


LayoutOperator::LayoutOperator() 
{
}	


int LayoutOperator::GetNumChildExpressions() const
{
	return child_expressions.size();
}


LayoutOperator::ChildExpressionIterator LayoutOperator::GetChildrenBegin()
{
	return child_expressions.begin();
}


LayoutOperator::ChildExpressionIterator LayoutOperator::GetChildrenEnd()
{
	return child_expressions.end();
}


list<shared_ptr<Layout>> &LayoutOperator::GetChildExpressions() 
{
	return child_expressions;
}


const list<shared_ptr<Layout>> &LayoutOperator::GetChildExpressions() const
{
	return child_expressions;
}


list<shared_ptr<Layout>> &&LayoutOperator::MoveChildExpressions()
{
	return move(child_expressions);
}


string LayoutOperator::GetChildExpressionsTrace() const
{
	return Trace(child_expressions);
}


void LayoutOperator::ForChildren(function<void(shared_ptr<Layout> &expr)> func) try
{
	for( shared_ptr<Layout> &child_expression : child_expressions )
		func(child_expression);
}
catch( BreakException )
{
}		       


void LayoutOperator::DepthFirstWalkImpl( function<void(shared_ptr<Layout> &expr)> func_in,
			                             function<void(shared_ptr<Layout> &expr)> func_out )
{
	for( shared_ptr<Layout> &expr : child_expressions )
	{
		if( func_in )
			func_in(expr);
		expr->DepthFirstWalkImpl(func_in, func_out);
		if( func_out )
			func_out(expr);
	}
}

// ------------------------- MergeZoneOperator --------------------------

MergeZoneOperator::MergeZoneOperator( list<shared_ptr<Layout>> &&child_expressions_ ) :
	LayoutOperator(move(child_expressions_))
{
}	


MergeZoneOperator::MergeZoneOperator() 
{
}	


void MergeZoneOperator::AddEmbeddedMarker( RequiresSubordinateSCREngine *new_marker )
{
	AddEmbeddedMarkers( { new_marker } );
}

// ------------------------- DupMergeTreeZoneOperator --------------------------

DupMergeTreeZoneOperator::DupMergeTreeZoneOperator( TreeZone zone_, 
                                                    list<shared_ptr<Layout>> &&child_expressions ) :
	MergeZoneOperator( move(child_expressions) ),
	zone(zone_)
{
	ASSERT( zone.GetNumTerminii() == GetNumChildExpressions() );	
}	
		

DupMergeTreeZoneOperator::DupMergeTreeZoneOperator( TreeZone zone_ ) :
	MergeZoneOperator(),
	zone(zone_)
{
	// If zone has terminii, they will be "exposed".
}


void DupMergeTreeZoneOperator::AddEmbeddedMarkers( list<RequiresSubordinateSCREngine *> &&new_markers )
{
	embedded_markers.splice( embedded_markers.end(), move(new_markers) );
}


list<RequiresSubordinateSCREngine *> DupMergeTreeZoneOperator::GetEmbeddedMarkers() const
{
	return embedded_markers;
}


void DupMergeTreeZoneOperator::ClearEmbeddedMarkers()
{
	embedded_markers.clear();
}


TreeZone &DupMergeTreeZoneOperator::GetZone() 
{
    return zone;
}


const TreeZone &DupMergeTreeZoneOperator::GetZone() const
{
    return zone;
}


shared_ptr<Layout> DupMergeTreeZoneOperator::DuplicateToFree() const
{
	FreeZone free_zone = zone.Duplicate();
	list<shared_ptr<Layout>> c = GetChildExpressions();
	auto pop_fz_op = make_shared<MergeFreeZoneOperator>( free_zone, move(c) );
	pop_fz_op->AddEmbeddedMarkers( GetEmbeddedMarkers() );
	return pop_fz_op;
}	


string DupMergeTreeZoneOperator::GetTrace() const
{
#ifdef RECURSIVE_TRACE_OPERATOR
	return "DupMergeTreeZoneOperator( \nzone: "+Trace(zone)+",\nchildren: "+GetChildExpressionsTrace()+" )";
#else
	return "DupMergeTreeZoneOperator( zone: "+Trace(zone)+", "+Trace(GetNumChildExpressions())+" children )";
#endif
}

// ------------------------- MergeFreeZoneOperator --------------------------

MergeFreeZoneOperator::MergeFreeZoneOperator( FreeZone zone_, 
                                                    list<shared_ptr<Layout>> &&child_expressions ) :
	MergeZoneOperator( move(child_expressions) ),
	zone(zone_)
{
	ASSERT( zone.GetNumTerminii() == GetNumChildExpressions() );	
}

		
MergeFreeZoneOperator::MergeFreeZoneOperator( FreeZone zone_ ) :
   	MergeZoneOperator(),
   	zone(zone_)
{
	// If zone has terminii, they will be "exposed" and will remain 
	// in the zone returned by Evaluate.
}


void MergeFreeZoneOperator::AddEmbeddedMarkers( list<RequiresSubordinateSCREngine *> &&new_markers )
{
	// Rule #726 requires us to mark free zones immediately
	for( RequiresSubordinateSCREngine *ea : new_markers )
		zone.MarkBaseForEmbedded(ea);	
}


list<RequiresSubordinateSCREngine *> MergeFreeZoneOperator::GetEmbeddedMarkers() const
{
	return {}; // Rule #726 means there aren't any
}


void MergeFreeZoneOperator::ClearEmbeddedMarkers()
{
	// Rule #726 means there aren't any
}


MergeZoneOperator::ChildExpressionIterator MergeFreeZoneOperator::SpliceOver( ChildExpressionIterator it_child, 
                                                                                    list<shared_ptr<Layout>> &&child_exprs )
{
	// it_child updated to the next child after the one we erased, or end()
	it_child = GetChildExpressions().erase( it_child );		
	
	// Insert the child before it_child, i.e. where we just
	// erase()d from. I assume it_child now points after the inserted 
	// child, i.e. at the same element it did after the erase()
	GetChildExpressions().splice( it_child, move(child_exprs) );	
	
	return it_child;
}                                               


FreeZone &MergeFreeZoneOperator::GetZone()
{
    return zone;
}


const FreeZone &MergeFreeZoneOperator::GetZone() const
{
    return zone;
}


string MergeFreeZoneOperator::GetTrace() const
{
#ifdef RECURSIVE_TRACE_OPERATOR
	return "MergeFreeZoneOperator( \nzone: "+Trace(zone)+",\nchildren: "+GetChildExpressionsTrace()+" )";
#else
	return "MergeFreeZoneOperator( zone: "+Trace(zone)+", "+Trace(GetNumChildExpressions())+" children )";
#endif	
}

// ------------------------- ReplaceOperator --------------------------

ReplaceOperator::ReplaceOperator( TreeZone target_tree_zone_, 
                                  shared_ptr<Zone> source_zone_,
                                  list<shared_ptr<Layout>> &&child_expressions ) :
	LayoutOperator( move(child_expressions) ),
	target_tree_zone(target_tree_zone_),
	source_zone(source_zone_)
{
	ASSERT( target_tree_zone.GetNumTerminii() == source_zone->GetNumTerminii() );
	ASSERT( target_tree_zone.GetNumTerminii() == GetNumChildExpressions() );	
}


const TreeZone &ReplaceOperator::GetTargetTreeZone() const
{
	return target_tree_zone;
}


shared_ptr<Zone> ReplaceOperator::GetSourceZone() const
{
	return source_zone;
}

