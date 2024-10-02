#include "fz_expressions.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tree_update.hpp"

using namespace SR;

#define RECURSIVE_TRACE_OPERATOR

// ------------------------- FreeZoneExpression --------------------------

void FreeZoneExpression::ForDepthFirstWalk( shared_ptr<FreeZoneExpression> &base,
											function<void(shared_ptr<FreeZoneExpression> &expr)> func_in,
	                                        function<void(shared_ptr<FreeZoneExpression> &expr)> func_out ) try
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

PopulateZoneOperator::PopulateZoneOperator( const SCREngine *scr_engine_, list<shared_ptr<FreeZoneExpression>> &&child_expressions_ ) :
	scr_engine( scr_engine_ ),
	child_expressions(move(child_expressions_))
{
}	


PopulateZoneOperator::PopulateZoneOperator( const SCREngine *scr_engine_ ) :
	scr_engine( scr_engine_ )
{
}	


void PopulateZoneOperator::AddEmbeddedMarker( RequiresSubordinateSCREngine *new_marker )
{
	AddEmbeddedMarkers( { new_marker } );
}


void PopulateZoneOperator::AddEmbeddedMarkers( list<RequiresSubordinateSCREngine *> &&new_markers )
{
	embedded_markers.splice( embedded_markers.end(), move(new_markers) );
}


list<RequiresSubordinateSCREngine *> PopulateZoneOperator::GetEmbeddedMarkers() const
{
	return embedded_markers;
}


int PopulateZoneOperator::GetNumChildExpressions() const
{
	return child_expressions.size();
}


list<shared_ptr<FreeZoneExpression>> &PopulateZoneOperator::GetChildExpressions() 
{
	return child_expressions;
}


const list<shared_ptr<FreeZoneExpression>> &PopulateZoneOperator::GetChildExpressions() const
{
	return child_expressions;
}


string PopulateZoneOperator::GetChildExpressionsTrace() const
{
	return Trace(child_expressions);
}


void PopulateZoneOperator::ForChildren(function<void(shared_ptr<FreeZoneExpression> &expr)> func) try
{
	for( shared_ptr<FreeZoneExpression> &child_expression : child_expressions )
		func(child_expression);
}
catch( BreakException )
{
}		       


void PopulateZoneOperator::EvaluateWithFreeZone( FreeZone &free_zone ) const	
{
	//FTRACE(free_zone)("\n");
	list<unique_ptr<FreeZone>> child_zones;
	for( const shared_ptr<FreeZoneExpression> &child_expression : child_expressions )
	{	
		//FTRACE(child_expression)("\n");
		unique_ptr<FreeZone> free_zone = child_expression->Evaluate();
		child_zones.push_back( move(free_zone) );
	}
	
	free_zone.PopulateAll(move(child_zones));
	
	for( RequiresSubordinateSCREngine *ea : embedded_markers )
		free_zone.MarkBaseForEmbedded(scr_engine, ea);	
}
	

void PopulateZoneOperator::DepthFirstWalkImpl( function<void(shared_ptr<FreeZoneExpression> &expr)> func_in,
			                                   function<void(shared_ptr<FreeZoneExpression> &expr)> func_out )
{
	for( shared_ptr<FreeZoneExpression> &expr : child_expressions )
	{
		if( func_in )
			func_in(expr);
		expr->DepthFirstWalkImpl(func_in, func_out);
		if( func_out )
			func_out(expr);
	}
}
	
// ------------------------- PopulateTreeZoneOperator --------------------------

PopulateTreeZoneOperator::PopulateTreeZoneOperator( const SCREngine *scr_engine,
                                                    TreeZone zone_, 
                                                    list<shared_ptr<FreeZoneExpression>> &&child_expressions ) :
	PopulateZoneOperator( scr_engine, move(child_expressions) ),
	zone(zone_)
{
	ASSERT( zone.GetNumTerminii() == GetNumChildExpressions() );	
}	
		

PopulateTreeZoneOperator::PopulateTreeZoneOperator( const SCREngine *scr_engine,
                                                    TreeZone zone_ ) :
	PopulateZoneOperator( scr_engine ),
	zone(zone_)
{
	ASSERT( zone.GetNumTerminii() == 0 );	
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
	EvaluateWithFreeZone( *temp_free_zone );
	return temp_free_zone;
}


shared_ptr<FreeZoneExpression> PopulateTreeZoneOperator::DuplicateToFree() const
{
	FreeZone free_zone = zone.Duplicate();
	list<shared_ptr<FreeZoneExpression>> c = GetChildExpressions();
	auto pop_fz_op = make_shared<PopulateFreeZoneOperator>( scr_engine, free_zone, move(c) );
	pop_fz_op->AddEmbeddedMarkers( GetEmbeddedMarkers() );
	return pop_fz_op;
}	


string PopulateTreeZoneOperator::GetTrace() const
{
#ifdef RECURSIVE_TRACE_OPERATOR
	return "PopulateTreeZoneOperator( \nzone: "+Trace(zone)+",\nchildren: "+GetChildExpressionsTrace()+" )";
#else
	return "PopulateFreeZoneOperator( zone: "+Trace(zone)+", "+Trace(GetNumChildExpressions())+" children )";
#endif
}

// ------------------------- PopulateFreeZoneOperator --------------------------

PopulateFreeZoneOperator::PopulateFreeZoneOperator( const SCREngine *scr_engine,
                                                    FreeZone zone_, 
                                                    list<shared_ptr<FreeZoneExpression>> &&child_expressions ) :
	PopulateZoneOperator( scr_engine, move(child_expressions) ),
	zone(zone_)
{
	ASSERT( zone.GetNumTerminii() == GetNumChildExpressions() );	
}

		
PopulateFreeZoneOperator::PopulateFreeZoneOperator( const SCREngine *scr_engine,
                                                    FreeZone zone_ ) :
   	PopulateZoneOperator( scr_engine ),
   	zone(zone_)
{
	ASSERT( zone.GetNumTerminii() == 0 );	
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
	EvaluateWithFreeZone( *temp_free_zone );
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
