#include "teleport_agent.hpp"

#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "scr_engine.hpp"
#include "and_rule_engine.hpp"
#include "link.hpp"

// Temporary
#include "tree/cpptree.hpp"

#include "transform_of_agent.hpp"
#include "sym/lambdas.hpp"
#include "sym/boolean_operators.hpp"
#include "sym/predicate_operators.hpp"
#include "sym/primary_expressions.hpp"
#include "sym/clutch.hpp"

#include <stdexcept>

using namespace SR;
using namespace SYM;

//---------------------------------- TeleportAgent ------------------------------------    

void TeleportAgent::RunDecidedQueryPRed( DecidedQueryAgentInterface &query,
                                         XLink keyer_xlink ) const
{
    INDENT("T");
    
    auto op = [&](XLink keyer_xlink) -> map<PatternLink, XLink>
    {
        map<PatternLink, XLink> tp_links = RunTeleportQuery( keyer_xlink );
        
        // We will uniquify the link against the domain and then cache it against keyer_xlink
        
        for( pair<PatternLink, XLink> p : tp_links )
            p.second = master_scr_engine->UniquifyDomainExtension(p.second); // in-place
                   
        return tp_links;
    };
    
    map<PatternLink, XLink> cached_links = cache( keyer_xlink, op );
    for( LocatedLink cached_link : cached_links )
    {   
        ASSERT( cached_link );
        query.RegisterNormalLink( (PatternLink)cached_link, (XLink)cached_link );
    }    
}                                    


set<XLink> TeleportAgent::ExpandNormalDomain( const unordered_set<XLink> &keyer_xlinks )
{
    set<XLink> extra_xlinks;
    for( XLink keyer_xlink : keyer_xlinks )
    {
        if( keyer_xlink == XLink::MMAX_Link )
            continue; // MMAX at base never expands domain because all child patterns are also MMAX
        if( !IsPreRestrictionMatch(keyer_xlink) )
            continue; // Failed pre-restriction so can't expand domain

        try
        {
            shared_ptr<DecidedQuery> query = CreateDecidedQuery();
            RunDecidedQueryPRed( *query, keyer_xlink );
           
            for( LocatedLink extra_link : query->GetNormalLinks() )
                extra_xlinks.insert( (XLink)extra_link );
        }
        catch( ::Mismatch & ) {}
    }
    return extra_xlinks;
}


void TeleportAgent::Reset()
{
    AgentCommon::Reset();
    cache.Reset();
}

