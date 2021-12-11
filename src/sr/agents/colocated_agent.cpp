#include "colocated_agent.hpp"
#include "conjecture.hpp"
#include "link.hpp"
#include "sym/lambdas.hpp"
#include "sym/boolean_operators.hpp"
#include "sym/comparison_operators.hpp"
#include "sym/primary_expressions.hpp"

using namespace SR;
using namespace SYM;

void ColocatedAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                          XLink keyer_xlink ) const
{     
    if( keyer_xlink != XLink::MMAX_Link )
    {
        if( !IsLocalMatch( keyer_xlink.GetChildX().get() ) ) 
            throw PreRestrictionMismatch();

        RunColocatedQuery(keyer_xlink);
    }
    
    for( PatternLink plink : pattern_query->GetNormalLinks() )                 
        query.RegisterNormalLink( plink, keyer_xlink ); // Link into X
}    


bool ColocatedAgent::ImplHasNLQ() const
{    
    return true;
}


void ColocatedAgent::RunNormalLinkedQueryImpl( const SolutionMap *hypothesis_links,
                                               const TheKnowledge *knowledge ) const
{
    ASSERTFAIL("NLQ not implemented, use symbolic query instead");
}                            


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
                                                TreePtr<Node> key_node ) 
{
    auto plinks = pattern_query->GetNormalLinks();
    PatternLink replace_plink = plinks.front();
    ASSERT( replace_plink );          
    return replace_plink.GetChildAgent()->BuildReplace(replace_plink);
}


