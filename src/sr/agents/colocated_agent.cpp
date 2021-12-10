#include "colocated_agent.hpp"
#include "conjecture.hpp"
#include "link.hpp"
#include "sym/lambdas.hpp"
#include "sym/boolean_operators.hpp"
#include "sym/primary_expressions.hpp"

using namespace SR;

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
    // Baseless query strategy: symmetrical

    XLink prev_xlink;
    for( PatternLink plink : keyer_and_normal_plinks )   // loop over required plinks              
    {
        if( hypothesis_links->count(plink) == 1 )
        {
            XLink xlink = hypothesis_links->at(plink);
            if( !prev_xlink )   
            {         
                prev_xlink = xlink;
            }
            else
            {
                if( xlink != prev_xlink )
                {
                    ColocationMismatch e; // value of links mismatches
#ifdef HINTS_IN_EXCEPTIONS
                    e.hint = LocatedLink( plink, prev_xlink );
#endif           
                    throw e;                    
                }
            }
        }
    };
    
    if( !prev_xlink )
        return; // disjoint query (no overlap between hypothesis and required plinks)
    
    if( hypothesis_links->count(keyer_plink) == 1 )
    {
        XLink keyer_xlink = hypothesis_links->at(keyer_plink);
        
        // Now that the common xlink is known to be really common,
        // we can apply the usual checks including PR check and allowing for MMAX
        if( keyer_xlink == XLink::MMAX_Link )
            return;

        if( !IsLocalMatch( keyer_xlink.GetChildX().get() ) ) 
            throw PreRestrictionMismatch();
        
        RunColocatedQuery(keyer_xlink);    
    }
}                            


SYM::Lazy<SYM::BooleanExpression> ColocatedAgent::SymbolicNormalLinkedQuery() const
{
	// The keyer and normal children
	set<PatternLink> nlq_plinks = ToSetSolo( keyer_and_normal_plinks );
	auto nlq_lambda = [this](const SYM::Expression::EvalKit &kit)
	{
		XLink prev_xlink;
		for( PatternLink plink : keyer_and_normal_plinks )   // loop over required plinks              
		{
			if( kit.hypothesis_links->count(plink) == 1 )
			{
				XLink xlink = kit.hypothesis_links->at(plink);
				if( !prev_xlink )   
				{         
					prev_xlink = xlink;
				}
				else
				{
					if( xlink != prev_xlink )
					{
						ColocationMismatch e; // value of links mismatches
#ifdef HINTS_IN_EXCEPTIONS
						e.hint = LocatedLink( plink, prev_xlink );
#endif           
						throw e;                    
					}
				}
			}
		};
		
		if( !prev_xlink )
			return; // disjoint query (no overlap between hypothesis and required plinks)
		
		if( kit.hypothesis_links->count(keyer_plink) == 1 )
		{
			XLink keyer_xlink = kit.hypothesis_links->at(keyer_plink);
			
			// Now that the common xlink is known to be really common,
			// we can apply the usual checks including PR check and allowing for MMAX
			if( keyer_xlink == XLink::MMAX_Link )
				return;

			if( !IsLocalMatch( keyer_xlink.GetChildX().get() ) ) 
				throw PreRestrictionMismatch();
			
			//RunColocatedQuery(keyer_xlink);    
		}
	};
	return SYM::MakeLazy<SYM::BooleanLambda>(nlq_plinks, nlq_lambda, GetTrace()+".arb()") &
		   SymbolicColocatedQuery();
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


