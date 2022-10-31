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


TreePtr<Node> ColocatedAgent::BuildReplaceImpl( PatternLink me_plink, 
                                                XLink key_xlink ) 
{
	// Colocated agents forward to a child agent so they take up no 
	// space in generated tree. Therefore they don't use their keys.
	return BuildReplaceImpl(me_plink);
}


TreePtr<Node> ColocatedAgent::BuildReplaceImpl( PatternLink me_plink )
{
	// We use OnlyElementOf() as default impl because otherwise 
	// it's not clear which of multiple child links should be used for 
	// replace.
    auto plinks = pattern_query->GetNormalLinks();
    PatternLink replace_plink = OnlyElementOf(plinks);
    ASSERT( replace_plink );          
    return replace_plink.GetChildAgent()->BuildReplace(replace_plink);
}

