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


bool ColocatedAgent::ImplHasNLQ() const
{    
    return true;
}

    
bool ColocatedAgent::NLQRequiresBase() const
{
    return false;
}                                         


void ColocatedAgent::RunNormalLinkedQueryImpl( const SolutionMap *required_links,
                                               const TheKnowledge *knowledge ) const
{
    // Baseless query strategy: symmetrical

    XLink common_xlink;
    auto lambda_find_common = [&](PatternLink plink)
    {
        if( required_links->count(plink) == 0 )
            return; // Partial query: XLink not supplied
        
        XLink xlink = required_links->at(plink);
        if( !common_xlink )
            common_xlink = xlink;
    };
    
    auto lambda_enforce_colocation = [&](PatternLink plink)
    {
        if( required_links->count(plink) == 0 )
            return; // Partial query: XLink not supplied
        
        XLink xlink = required_links->at(plink);
        if( xlink != common_xlink )
        {
            ColocationMismatch e; // value of links mismatches
#ifdef HINTS_IN_EXCEPTIONS
            e.hint = LocatedLink( plink, common_xlink );
#endif           
            throw e;                    
        }
    };
    
    // Determine common xlink (giving priority to base)
    lambda_find_common( base_plink );
    for( PatternLink plink : pattern_query->GetNormalLinks() )                 
        lambda_find_common( plink );         
    ASSERT( common_xlink ); // empty query
    
    // Enforce colocation. Not need to check base since it
    // got priority for _being_ the common xlink.
    for( PatternLink plink : pattern_query->GetNormalLinks() )                 
        lambda_enforce_colocation( plink );         

    // Now that the common xlink is known to be really common,
    // we can apply the usual checks including PR check and allowing for MMAX
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


