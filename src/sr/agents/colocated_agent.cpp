#include "colocated_agent.hpp"
#include "conjecture.hpp"
#include "link.hpp"
#include "sym/boolean_operators.hpp"
#include "sym/predicate_operators.hpp"
#include "sym/symbol_operators.hpp"
#include "up/fz_expressions.hpp"

using namespace SR;
using namespace SYM;

//---------------------------------- ColocatedAgent ------------------------------------    

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
	return SYM::MakeLazy<SYM::BooleanConstant>(true); // TODO don't like this, belongs in ConjuctionAgent
}


Agent::ReplaceExprPtr ColocatedAgent::GenReplaceExprImpl( const ReplaceKit &kit, 
                                                            PatternLink me_plink, 
                                                            XLink key_xlink )
{
    auto plinks = pattern_query->GetNormalLinks();
    if( plinks.size() == 1 )
    {
		// Unambiguous path through replace pattern so we can continue to overlay
		PatternLink replace_plink = OnlyElementOf(plinks);
		ASSERT( replace_plink );          
		return replace_plink.GetChildAgent()->GenReplaceExpr(kit, replace_plink);    
	}
	else
	{
		return AgentCommon::GenReplaceExprImpl(kit, me_plink, key_xlink);
	}
}                                         
