#include "fz_expressions.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tree_update.hpp"

using namespace SR;

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


void FreeZoneExpression::DepthFirstWalkImpl( function<void(shared_ptr<FreeZoneExpression> &expr)> func_in,
			                                 function<void(shared_ptr<FreeZoneExpression> &expr)> func_out )
{
	ForChildren( [&](shared_ptr<FreeZoneExpression> &expr)
	{
		if( func_in )
			func_in(expr);
		expr->DepthFirstWalkImpl(func_in, func_out);
		if( func_out )
			func_out(expr);
	} );
}

// ------------------------- PopulateZoneOperator --------------------------

PopulateZoneOperator::PopulateZoneOperator( vector<shared_ptr<FreeZoneExpression>> &&child_expressions_ ) :
	child_expressions(move(child_expressions_))
{
}	


PopulateZoneOperator::PopulateZoneOperator()
{
}	


void PopulateZoneOperator::AddEmbeddedMarker( RequiresSubordinateSCREngine *new_marker )
{
	embedded_markers.push_back( new_marker );
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


vector<shared_ptr<FreeZoneExpression>> &PopulateZoneOperator::GetChildExpressions() 
{
	return child_expressions;
}


const vector<shared_ptr<FreeZoneExpression>> &PopulateZoneOperator::GetChildExpressions() const
{
	return child_expressions;
}


string PopulateZoneOperator::GetChildExpressionsTrace() const
{
	return Trace(child_expressions);
}


void PopulateZoneOperator::ForChildren(function<void(shared_ptr<FreeZoneExpression> &expr)> func)
{
	for( shared_ptr<FreeZoneExpression> &child_expression : child_expressions )
		func(child_expression);
}


void PopulateZoneOperator::PopulateFreeZone( FreeZone &free_zone, const UP::ExecKit &kit ) const	
{
	//FTRACE(free_zone)("\n");
	vector<unique_ptr<FreeZone>> child_zones;
	for( const shared_ptr<FreeZoneExpression> &child_expression : child_expressions )
	{
		//FTRACE(child_expression)("\n");
		unique_ptr<FreeZone> free_zone = child_expression->Evaluate( kit );
		child_zones.push_back( move(free_zone) );
	}
	
	free_zone.Populate(kit.x_tree_db, move(child_zones));
	
	for( RequiresSubordinateSCREngine *ea : embedded_markers )
		free_zone.MarkBaseForEmbedded(kit.scr_engine, ea);		
}
	
// ------------------------- PopulateTreeZoneOperator --------------------------

PopulateTreeZoneOperator::PopulateTreeZoneOperator( TreeZone zone_, 
                                                    vector<shared_ptr<FreeZoneExpression>> &&child_expressions ) :
	PopulateZoneOperator( move(child_expressions) ),
	zone(zone_)
{
	ASSERT( zone.GetNumTerminii() == GetNumChildExpressions() );	
}	
		

PopulateTreeZoneOperator::PopulateTreeZoneOperator( TreeZone zone_ ) :
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


unique_ptr<FreeZone> PopulateTreeZoneOperator::Evaluate( const UP::ExecKit &kit ) const
{
	// TODO probably consistent for Duplicate() to return unique_ptr<FreeZone>
	auto temp_free_zone = make_unique<FreeZone>( zone.Duplicate() );
	PopulateFreeZone( *temp_free_zone, kit );
	return temp_free_zone;
}


shared_ptr<FreeZoneExpression> PopulateTreeZoneOperator::DuplicateToFree() const
{
	FreeZone free_zone = zone.Duplicate();
	vector<shared_ptr<FreeZoneExpression>> c = GetChildExpressions();
	auto pop_fz_op = make_shared<PopulateFreeZoneOperator>( free_zone, move(c) );
	pop_fz_op->AddEmbeddedMarkers( GetEmbeddedMarkers() );
	return pop_fz_op;
}	


string PopulateTreeZoneOperator::GetTrace() const
{
	return "PopulateTreeZoneOperator( \nzone: "+Trace(zone)+",\nchildren: "+GetChildExpressionsTrace()+" )";
}

// ------------------------- PopulateFreeZoneOperator --------------------------

PopulateFreeZoneOperator::PopulateFreeZoneOperator( FreeZone zone_, 
                                                    vector<shared_ptr<FreeZoneExpression>> &&child_expressions ) :
	PopulateZoneOperator( move(child_expressions) ),
	zone(zone_)
{
	ASSERT( zone.GetNumTerminii() == GetNumChildExpressions() );	
}

		
PopulateFreeZoneOperator::PopulateFreeZoneOperator( FreeZone zone_ ) :
   	PopulateZoneOperator(),
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


unique_ptr<FreeZone> PopulateFreeZoneOperator::Evaluate( const UP::ExecKit &kit ) const
{
	auto temp_free_zone = make_unique<FreeZone>(zone);
	PopulateFreeZone( *temp_free_zone, kit );
	return temp_free_zone;
}


string PopulateFreeZoneOperator::GetTrace() const
{
	return "PopulateFreeZoneOperator( \nzone: "+Trace(zone)+",\nchildren: "+GetChildExpressionsTrace()+" )";
}
