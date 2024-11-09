#include "zone_expressions.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tree_update.hpp"

using namespace SR;

//#define RECURSIVE_TRACE_OPERATOR

// ------------------------- ZoneExpression --------------------------

void ZoneExpression::ForDepthFirstWalk( shared_ptr<ZoneExpression> &base,
											function<void(shared_ptr<ZoneExpression> &expr)> func_in,
	                                        function<void(shared_ptr<ZoneExpression> &expr)> func_out ) try
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

// ------------------------- MergeZoneOperator --------------------------

MergeZoneOperator::MergeZoneOperator( list<shared_ptr<ZoneExpression>> &&child_expressions_ ) :
	child_expressions(move(child_expressions_))
{
}	


MergeZoneOperator::MergeZoneOperator() 
{
}	


void MergeZoneOperator::AddEmbeddedMarker( RequiresSubordinateSCREngine *new_marker )
{
	AddEmbeddedMarkers( { new_marker } );
}


int MergeZoneOperator::GetNumChildExpressions() const
{
	return child_expressions.size();
}


MergeZoneOperator::ChildExpressionIterator MergeZoneOperator::GetChildrenBegin()
{
	return child_expressions.begin();
}


MergeZoneOperator::ChildExpressionIterator MergeZoneOperator::GetChildrenEnd()
{
	return child_expressions.end();
}


list<shared_ptr<ZoneExpression>> &MergeZoneOperator::GetChildExpressions() 
{
	return child_expressions;
}


const list<shared_ptr<ZoneExpression>> &MergeZoneOperator::GetChildExpressions() const
{
	return child_expressions;
}


list<shared_ptr<ZoneExpression>> &&MergeZoneOperator::MoveChildExpressions()
{
	return move(child_expressions);
}


string MergeZoneOperator::GetChildExpressionsTrace() const
{
	return Trace(child_expressions);
}


void MergeZoneOperator::ForChildren(function<void(shared_ptr<ZoneExpression> &expr)> func) try
{
	for( shared_ptr<ZoneExpression> &child_expression : child_expressions )
		func(child_expression);
}
catch( BreakException )
{
}		       


void MergeZoneOperator::EvaluateChildrenAndPopulate( const UpEvalExecKit &kit, FreeZone &free_zone ) const	
{
	// If no child expressions then either:
	// - zone has no terminii and we're at a subtree -> no action required
	// - zone's terminii are "exposed" and should be left in place
	if( child_expressions.empty() )
		return;
	
	//FTRACE(free_zone)("\n");
	list<unique_ptr<FreeZone>> child_zones;
	for( const shared_ptr<ZoneExpression> &child_expression : child_expressions )
	{	
		//FTRACE(child_expression)("\n");
		unique_ptr<FreeZone> free_zone = child_expression->Evaluate(kit);
		child_zones.push_back( move(free_zone) );
	}
	
	free_zone.MergeAll(move(child_zones));
}
	

void MergeZoneOperator::DepthFirstWalkImpl( function<void(shared_ptr<ZoneExpression> &expr)> func_in,
			                                   function<void(shared_ptr<ZoneExpression> &expr)> func_out )
{
	for( shared_ptr<ZoneExpression> &expr : child_expressions )
	{
		if( func_in )
			func_in(expr);
		expr->DepthFirstWalkImpl(func_in, func_out);
		if( func_out )
			func_out(expr);
	}
}
	
// ------------------------- DupMergeTreeZoneOperator --------------------------

DupMergeTreeZoneOperator::DupMergeTreeZoneOperator( TreeZone zone_, 
                                                    list<shared_ptr<ZoneExpression>> &&child_expressions ) :
	MergeZoneOperator( move(child_expressions) ),
	zone(zone_)
{
	ASSERT( zone.GetNumTerminii() == GetNumChildExpressions() );	
}	
		

DupMergeTreeZoneOperator::DupMergeTreeZoneOperator( TreeZone zone_ ) :
	MergeZoneOperator(),
	zone(zone_)
{
	// If zone has terminii, they will be "exposed" and will appear  
	// in the free zone returned by Evaluate.
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


unique_ptr<FreeZone> DupMergeTreeZoneOperator::Evaluate(const UpEvalExecKit &kit) const
{
	// TODO probably consistent for Duplicate() to return unique_ptr<FreeZone>
	auto temp_free_zone = make_unique<FreeZone>( zone.Duplicate() );

	// Rule #726 now we've gone to free zone, mark immediately.
	for( RequiresSubordinateSCREngine *ea : embedded_markers )
		temp_free_zone->MarkBaseForEmbedded(ea);		
	
	EvaluateChildrenAndPopulate( kit, *temp_free_zone );
	return temp_free_zone;
}


shared_ptr<ZoneExpression> DupMergeTreeZoneOperator::DuplicateToFree() const
{
	FreeZone free_zone = zone.Duplicate();
	list<shared_ptr<ZoneExpression>> c = GetChildExpressions();
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
                                                    list<shared_ptr<ZoneExpression>> &&child_expressions ) :
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
                                                                                    list<shared_ptr<ZoneExpression>> &&child_exprs )
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


unique_ptr<FreeZone> MergeFreeZoneOperator::Evaluate(const UpEvalExecKit &kit) const
{
	auto temp_free_zone = make_unique<FreeZone>(zone);
	EvaluateChildrenAndPopulate( kit, *temp_free_zone );
	return temp_free_zone;
}


string MergeFreeZoneOperator::GetTrace() const
{
#ifdef RECURSIVE_TRACE_OPERATOR
	return "MergeFreeZoneOperator( \nzone: "+Trace(zone)+",\nchildren: "+GetChildExpressionsTrace()+" )";
#else
	return "MergeFreeZoneOperator( zone: "+Trace(zone)+", "+Trace(GetNumChildExpressions())+" children )";
#endif	
}
