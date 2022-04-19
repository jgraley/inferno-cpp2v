#include "colocated_agent.hpp"
#include "conjecture.hpp"
#include "link.hpp"
#include "sym/boolean_operators.hpp"
#include "sym/predicate_operators.hpp"
#include "sym/symbol_operators.hpp"

using namespace SR;
using namespace SYM;

Over<BooleanExpression> ColocatedAgent::SymbolicNormalLinkedQuery() const
{
	Over<BooleanExpression> my_expr = SYM::MakeOver<SYM::BooleanConstant>(true);

	for( PatternLink plink : pattern_query->GetNormalLinks() )
		my_expr = my_expr & MakeOver<SymbolVariable>(keyer_plink) == MakeOver<SymbolVariable>(plink);
	
	my_expr = my_expr &
	          SymbolicColocatedQuery() &
   		      ( SymbolicPreRestriction() |
		        MakeOver<SymbolVariable>(keyer_plink) == MakeOver<SymbolConstant>(XLink::MMAX_Link) );

	return my_expr;
}


void ColocatedAgent::RunColocatedQuery( XLink common_xlink ) const
{
    // No restriction by default
}


SYM::Over<SYM::BooleanExpression> ColocatedAgent::SymbolicColocatedQuery() const
{
	return SYM::MakeOver<SYM::BooleanConstant>(true); 
}


TreePtr<Node> ColocatedAgent::BuildReplaceImpl( PatternLink me_plink, 
                                                TreePtr<Node> key_node ) 
{
    auto plinks = pattern_query->GetNormalLinks();
    PatternLink replace_plink = plinks.front();
    ASSERT( replace_plink );          
    return replace_plink.GetChildAgent()->BuildReplace(replace_plink);
}


