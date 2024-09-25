#include "fz_expressions.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tree_update.hpp"

using namespace SR;

// ------------------------- FreeZoneExpression --------------------------

void FreeZoneExpression::ForDepthFirstWalk( const FreeZoneExpression *base,
	                                        function<void(const FreeZoneExpression *cmd)> func_in,
	                                        function<void(const FreeZoneExpression *cmd)> func_out ) try
{
	base->DepthFirstWalkImpl(func_in, func_out);
}
catch( BreakException )
{
}		       

// ------------------------- PopulateZoneOperator --------------------------

PopulateZoneOperator::PopulateZoneOperator( vector<shared_ptr<FreeZoneExpression>> &&child_expressions_ ) :
	child_expressions(move(child_expressions_))
{
}	


PopulateZoneOperator::PopulateZoneOperator()
{
}	


void PopulateZoneOperator::AddEmbeddedMarker( RequiresSubordinateSCREngine *embedded_marker )
{
	embedded_markers.push_back( embedded_marker );
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


void PopulateZoneOperator::DepthFirstWalkImpl(function<void(const FreeZoneExpression *cmd)> func_in,
			                                  function<void(const FreeZoneExpression *cmd)> func_out) const
{
	if( func_in )
		func_in(this);
	for( const shared_ptr<FreeZoneExpression> &child_expression : child_expressions )
		child_expression->DepthFirstWalkImpl(func_in, func_out);
	if( func_out )
		func_out(this);
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


TreeZone *PopulateTreeZoneOperator::GetZone() 
{
    return &zone;
}


const TreeZone *PopulateTreeZoneOperator::GetZone() const
{
    return &zone;
}


unique_ptr<FreeZone> PopulateTreeZoneOperator::Evaluate( const UP::ExecKit &kit ) const
{
	// TODO probably consistent for Duplicate() to return unique_ptr<FreeZone>
	auto temp_free_zone = make_unique<FreeZone>( zone.Duplicate( kit.x_tree_db ) );
	PopulateFreeZone( *temp_free_zone, kit );
	return temp_free_zone;
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


FreeZone *PopulateFreeZoneOperator::GetZone()
{
    return &zone;
}


const FreeZone *PopulateFreeZoneOperator::GetZone() const
{
    return &zone;
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
