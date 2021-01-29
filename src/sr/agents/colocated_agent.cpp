#include "colocated_agent.hpp"
#include "conjecture.hpp"
#include "link.hpp"

using namespace SR;

void ColocatedAgent::RunDecidedQueryPRed( DecidedQueryAgentInterface &query,
                                          XLink base_xlink ) const
{ 
    INDENT("∧");
    query.Reset();
    auto plinks = pattern_query->GetNormalLinks();
    ASSERT( !plinks.empty() ); // must be at least one thing!

    RunColocatedQuery(base_xlink);

    for( PatternLink plink : plinks )                 
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


