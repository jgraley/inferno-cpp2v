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

PopulateZoneOperator::PopulateZoneOperator( vector<unique_ptr<FreeZoneExpression>> &&child_expressions_ ) :
	child_expressions(move(child_expressions_))
{
}	


PopulateZoneOperator::PopulateZoneOperator()
{
}	


void PopulateZoneOperator::AddEmbeddedAgentBase( RequiresSubordinateSCREngine *embedded_agent )
{
	embedded_agents.push_back( embedded_agent );
}


int PopulateZoneOperator::GetNumChildExpressions() const
{
	return child_expressions.size();
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
	for( const unique_ptr<FreeZoneExpression> &child_expression : child_expressions )
		child_expression->DepthFirstWalkImpl(func_in, func_out);
	if( func_out )
		func_out(this);
}


void PopulateZoneOperator::PopulateFreeZone( FreeZone &free_zone, const UP::ExecKit &kit ) const	
{
	//FTRACE(free_zone)("\n");
	vector<FreeZone> child_zones;
	for( const unique_ptr<FreeZoneExpression> &child_expression : child_expressions )
	{
		//FTRACE(child_expression)("\n");
		unique_ptr<Zone> zone = child_expression->Evaluate( kit );
		if( auto free_zone = dynamic_pointer_cast<FreeZone>(zone) )
			child_zones.push_back( *free_zone );
		else
			ASSERTFAIL(); // not a free zone	
	}
	
	free_zone.Populate(kit.x_tree_db, child_zones);
	
	for( RequiresSubordinateSCREngine *ea : embedded_agents )
		free_zone.MarkBaseForEmbedded(kit.scr_engine, ea);		
}
	
// ------------------------- PopulateTreeZoneOperator --------------------------

PopulateTreeZoneOperator::PopulateTreeZoneOperator( unique_ptr<TreeZone> &&zone_, 
                                                  vector<unique_ptr<FreeZoneExpression>> &&child_expressions ) :
	PopulateZoneOperator( move(child_expressions) ),
	zone(move(zone_))
{
	ASSERT( zone->GetNumTerminii() == GetNumChildExpressions() );	
}	
		

PopulateTreeZoneOperator::PopulateTreeZoneOperator( unique_ptr<TreeZone> &&zone_ ) :
	zone(move(zone_))
{
	ASSERT( zone->GetNumTerminii() == 0 );	
}


const TreeZone *PopulateTreeZoneOperator::GetZone() const
{
    return zone.get();
}


unique_ptr<Zone> PopulateTreeZoneOperator::Evaluate( const UP::ExecKit &kit ) const
{
	FreeZone free_zone = zone->Duplicate( kit.x_tree_db );
	PopulateFreeZone( free_zone, kit );
	return make_unique<FreeZone>(free_zone);
}


string PopulateTreeZoneOperator::GetTrace() const
{
	return "PopulateTreeZoneOperator( \nzone: "+Trace(zone)+",\nchildren: "+GetChildExpressionsTrace()+" )";
}

// ------------------------- PopulateFreeZoneOperator --------------------------

PopulateFreeZoneOperator::PopulateFreeZoneOperator( unique_ptr<FreeZone> &&zone_, 
                                                  vector<unique_ptr<FreeZoneExpression>> &&child_expressions ) :
	PopulateZoneOperator( move(child_expressions) ),
	zone(move(zone_))
{
	ASSERT( zone->GetNumTerminii() == GetNumChildExpressions() );	
}

		
PopulateFreeZoneOperator::PopulateFreeZoneOperator( unique_ptr<FreeZone> &&zone_ ) :
   	PopulateZoneOperator(),
   	zone(move(zone_))
{
	ASSERT( zone->GetNumTerminii() == 0 );	
}


const FreeZone *PopulateFreeZoneOperator::GetZone() const
{
    return zone.get();
}


unique_ptr<Zone> PopulateFreeZoneOperator::Evaluate( const UP::ExecKit &kit ) const
{
	PopulateFreeZone( *zone, kit );
	return make_unique<FreeZone>(*zone);
}


string PopulateFreeZoneOperator::GetTrace() const
{
	return "PopulateFreeZoneOperator( \nzone: "+Trace(zone)+",\nchildren: "+GetChildExpressionsTrace()+" )";
}
