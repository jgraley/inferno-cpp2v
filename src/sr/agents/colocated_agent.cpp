#include "colocated_agent.hpp"
#include "conjecture.hpp"
#include "link.hpp"

using namespace SR;

void ColocatedAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                          XLink base_xlink ) const
{ 
    INDENT("∧");
    
    if( base_xlink != XLink::MMAX_Link )
    {
        CheckLocalMatch( base_xlink.GetChildX().get() );
        RunColocatedQuery(base_xlink);
    }
    
    for( PatternLink plink : pattern_query->GetNormalLinks() )                 
        query.RegisterNormalLink( plink, base_xlink ); // Link into X
}    


void ColocatedAgent::RunColocatedQuery( XLink common_xlink ) const
{
    // No restriction by default
}


TreePtr<Node> ColocatedAgent::BuildReplaceImpl() 
{
    auto plinks = pattern_query->GetNormalLinks();
    PatternLink replace_plink = plinks.front();
    ASSERT( replace_plink );          
    return replace_plink.GetChildAgent()->BuildReplace();
}


