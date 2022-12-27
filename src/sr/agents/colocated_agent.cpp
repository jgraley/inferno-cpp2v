#include "colocated_agent.hpp"
#include "conjecture.hpp"
#include "link.hpp"
#include "sym/boolean_operators.hpp"
#include "sym/predicate_operators.hpp"
#include "sym/symbol_operators.hpp"

using namespace SR;
using namespace SYM;

Lazy<BooleanExpression> ColocatedAgent::SymbolicNormalLinkedQuery() const
{
	Lazy<BooleanExpression> my_expr = SYM::MakeLazy<SYM::BooleanConstant>(true);

	for( PatternLink plink : pattern_query->GetNormalLinks() )
		my_expr = my_expr & MakeLazy<SymbolVariable>(keyer_plink) == MakeLazy<SymbolVariable>(plink);
	
	my_expr = my_expr &
	          SymbolicColocatedQuery() &
   		      ( SymbolicPreRestriction() |
		        MakeLazy<SymbolVariable>(keyer_plink) == MakeLazy<SymbolConstant>(XLink::MMAX_Link) );

	return my_expr;
}


void ColocatedAgent::RunColocatedQuery( XLink common_xlink ) const
{
    // No restriction by default
}


SYM::Lazy<SYM::BooleanExpression> ColocatedAgent::SymbolicColocatedQuery() const
{
	return SYM::MakeLazy<SYM::BooleanConstant>(true); 
}
