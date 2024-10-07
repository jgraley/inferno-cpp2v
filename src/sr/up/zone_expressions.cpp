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

// ------------------------- PopulateZoneOperator --------------------------

PopulateZoneOperator::PopulateZoneOperator( list<shared_ptr<ZoneExpression>> &&child_expressions_ ) :
	child_expressions(move(child_expressions_))
{
}	


PopulateZoneOperator::PopulateZoneOperator() 
{
}	


void PopulateZoneOperator::AddEmbeddedMarker( RequiresSubordinateSCREngine *new_marker )
{
	AddEmbeddedMarkers( { new_marker } );
}


int PopulateZoneOperator::GetNumChildExpressions() const
{
	return child_expressions.size();
}


PopulateZoneOperator::ChildExpressionIterator PopulateZoneOperator::GetChildrenBegin()
{
	return child_expressions.begin();
}


PopulateZoneOperator::ChildExpressionIterator PopulateZoneOperator::GetChildrenEnd()
{
	return child_expressions.end();
}


list<shared_ptr<ZoneExpression>> &PopulateZoneOperator::GetChildExpressions() 
{
	return child_expressions;
}


const list<shared_ptr<ZoneExpression>> &PopulateZoneOperator::GetChildExpressions() const
{
	return child_expressions;
}


list<shared_ptr<ZoneExpression>> &&PopulateZoneOperator::MoveChildExpressions()
{
	return move(child_expressions);
}


string PopulateZoneOperator::GetChildExpressionsTrace() const
{
	return Trace(child_expressions);
}


void PopulateZoneOperator::ForChildren(function<void(shared_ptr<ZoneExpression> &expr)> func) try
{
	for( shared_ptr<ZoneExpression> &child_expression : child_expressions )
		func(child_expression);
}
catch( BreakException )
{
}		       


void PopulateZoneOperator::EvaluateChildrenAndPopulate( FreeZone &free_zone ) const	
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
		unique_ptr<FreeZone> free_zone = child_expression->Evaluate();
		child_zones.push_back( move(free_zone) );
	}
	
	free_zone.PopulateAll(move(child_zones));
}
	

void PopulateZoneOperator::DepthFirstWalkImpl( function<void(shared_ptr<ZoneExpression> &expr)> func_in,
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
	
// ------------------------- PopulateTreeZoneOperator --------------------------

PopulateTreeZoneOperator::PopulateTreeZoneOperator( TreeZone zone_, 
                                                    list<shared_ptr<ZoneExpression>> &&child_expressions ) :
	PopulateZoneOperator( move(child_expressions) ),
	zone(zone_)
{
	ASSERT( zone.GetNumTerminii() == GetNumChildExpressions() );	
}	
		

PopulateTreeZoneOperator::PopulateTreeZoneOperator( TreeZone zone_ ) :
	PopulateZoneOperator(),
	zone(zone_)
{
	// If zone has terminii, they will be "exposed" and will appear  
	// in the free zone returned by Evaluate.
}


void PopulateTreeZoneOperator::AddEmbeddedMarkers( list<RequiresSubordinateSCREngine *> &&new_markers )
{
	embedded_markers.splice( embedded_markers.end(), move(new_markers) );
}


list<RequiresSubordinateSCREngine *> PopulateTreeZoneOperator::GetEmbeddedMarkers() const
{
	return embedded_markers;
}


void PopulateTreeZoneOperator::ClearEmbeddedMarkers()
{
	embedded_markers.clear();
}


TreeZone &PopulateTreeZoneOperator::GetZone() 
{
    return zone;
}


const TreeZone &PopulateTreeZoneOperator::GetZone() const
{
    return zone;
}


unique_ptr<FreeZone> PopulateTreeZoneOperator::Evaluate() const
{
	// TODO probably consistent for Duplicate() to return unique_ptr<FreeZone>
	auto temp_free_zone = make_unique<FreeZone>( zone.Duplicate() );

	// Rule #726 now we've gone to free zone, mark immediately.
	for( RequiresSubordinateSCREngine *ea : embedded_markers )
		temp_free_zone->MarkBaseForEmbedded(ea);		
	
	EvaluateChildrenAndPopulate( *temp_free_zone );
	return temp_free_zone;
}


shared_ptr<ZoneExpression> PopulateTreeZoneOperator::DuplicateToFree() const
{
	FreeZone free_zone = zone.Duplicate();
	list<shared_ptr<ZoneExpression>> c = GetChildExpressions();
	auto pop_fz_op = make_shared<PopulateFreeZoneOperator>( free_zone, move(c) );
	pop_fz_op->AddEmbeddedMarkers( GetEmbeddedMarkers() );
	return pop_fz_op;
}	


string PopulateTreeZoneOperator::GetTrace() const
{
#ifdef RECURSIVE_TRACE_OPERATOR
	return "PopulateTreeZoneOperator( \nzone: "+Trace(zone)+",\nchildren: "+GetChildExpressionsTrace()+" )";
#else
	return "PopulateTreeZoneOperator( zone: "+Trace(zone)+", "+Trace(GetNumChildExpressions())+" children )";
#endif
}

// ------------------------- PopulateFreeZoneOperator --------------------------

PopulateFreeZoneOperator::PopulateFreeZoneOperator( FreeZone zone_, 
                                                    list<shared_ptr<ZoneExpression>> &&child_expressions ) :
	PopulateZoneOperator( move(child_expressions) ),
	zone(zone_)
{
	ASSERT( zone.GetNumTerminii() == GetNumChildExpressions() );	
}

		
PopulateFreeZoneOperator::PopulateFreeZoneOperator( FreeZone zone_ ) :
   	PopulateZoneOperator(),
   	zone(zone_)
{
	// If zone has terminii, they will be "exposed" and will remain 
	// in the zone returned by Evaluate.
}


void PopulateFreeZoneOperator::AddEmbeddedMarkers( list<RequiresSubordinateSCREngine *> &&new_markers )
{
	// Rule #726 requires us to mark free zones immediately
	for( RequiresSubordinateSCREngine *ea : new_markers )
		zone.MarkBaseForEmbedded(ea);	
}


list<RequiresSubordinateSCREngine *> PopulateFreeZoneOperator::GetEmbeddedMarkers() const
{
	return {}; // Rule #726 means there aren't any
}


void PopulateFreeZoneOperator::ClearEmbeddedMarkers()
{
	// Rule #726 means there aren't any
}


PopulateZoneOperator::ChildExpressionIterator PopulateFreeZoneOperator::SpliceOver( ChildExpressionIterator it_child, 
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


FreeZone &PopulateFreeZoneOperator::GetZone()
{
    return zone;
}


const FreeZone &PopulateFreeZoneOperator::GetZone() const
{
    return zone;
}


unique_ptr<FreeZone> PopulateFreeZoneOperator::Evaluate() const
{
	auto temp_free_zone = make_unique<FreeZone>(zone);
	EvaluateChildrenAndPopulate( *temp_free_zone );
	return temp_free_zone;
}


string PopulateFreeZoneOperator::GetTrace() const
{
#ifdef RECURSIVE_TRACE_OPERATOR
	return "PopulateFreeZoneOperator( \nzone: "+Trace(zone)+",\nchildren: "+GetChildExpressionsTrace()+" )";
#else
	return "PopulateFreeZoneOperator( zone: "+Trace(zone)+", "+Trace(GetNumChildExpressions())+" children )";
#endif	
}
