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
        if( !IsLocalMatch( base_xlink.GetChildX().get() ) ) 
            throw PreRestrictionMismatch();

        RunColocatedQuery(base_xlink);
    }
    
    for( PatternLink plink : pattern_query->GetNormalLinks() )                 
        query.RegisterNormalLink( plink, base_xlink ); // Link into X
}    


void ColocatedAgent::RunNormalLinkedQueryImpl( PatternLink base_plink,
                                               const SolutionMap *required_links,
                                               const TheKnowledge *knowledge ) const
{
    // Baseless query strategy: symmetrical
    PatternQuery::Links base_and_normal_plinks = pattern_query->GetNormalLinks();
    base_and_normal_plinks.push_front(base_plink);
    
    XLink common_xlink;
    for( PatternLink plink : base_and_normal_plinks )                 
    {
        if( required_links->count(plink) > 0 )
        {
            XLink xlink = required_links->at(plink);
            if( !common_xlink )
                common_xlink = xlink;
            else if( xlink != common_xlink )
                throw ColocationMismatch();
        }
    }            

    if( common_xlink != XLink::MMAX_Link )
    {
        if( !IsLocalMatch( common_xlink.GetChildX().get() ) ) 
            throw PreRestrictionMismatch();

        RunColocatedQuery(common_xlink);
    }    
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


