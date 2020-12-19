#include "match_any_agent.hpp"
#include "conjecture.hpp"
#include "link.hpp"

using namespace SR;

void MatchAnyAgent::AgentConfigure( Phase phase, const SCREngine *master_scr_engine )
{
    AgentCommon::AgentConfigure(phase, master_scr_engine);

    options = make_shared< Collection<Node> >();
    FOREACH( const TreePtrInterface &p, GetPatterns() )
        options->insert( p );
}


shared_ptr<PatternQuery> MatchAnyAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>(this);
    pq->RegisterDecision(false); // Exclusive, please
    for( CollectionInterface::iterator pit = GetPatterns().begin(); pit != GetPatterns().end(); ++pit )                 
    {
        const TreePtrInterface *p = &*pit; 
	    pq->RegisterNormalLink( PatternLink(this, p) );
    }
    
    return pq;
}


void MatchAnyAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                         XLink x ) const
{
    INDENT("|");
    ASSERT( !GetPatterns().empty() ); // must be at least one thing!
    query.Reset(); 
    
    // We register a decision that actually chooses between our agents - that
    // is, the options for the OR operation.
    ContainerInterface::iterator choice_it = query.RegisterDecision( options, false );
    FOREACH( const TreePtrInterface &p, GetPatterns() )                 
    {
        PatternLink plink(this, &p);

        // Context is normal because all patterns must match
        if( *plink.GetPatternPtr() == *choice_it ) // Is this the pattern that was chosen?
        {
            // Yes, so supply the "real" x for this link. We'll really
            // test x against this pattern.
            query.RegisterNormalLink( plink, x ); // Link into X
        }
        else
        {
            // No, so just make sure this link matches (overall AND-rule
            // applies, reducing the outcome to that of the normal 
            // link registered above).
            query.RegisterNormalLink( plink, XLink::MMAX_Link ); // Link into MMAX
        }
    }
}


bool MatchAnyAgent::ImplHasDNLQ() const
{
    return true;
}


Agent::Completeness MatchAnyAgent::RunDecidedNormalLinkedQueryImpl( DecidedQueryAgentInterface &query,
                                                                    XLink base_xlink,
                                                                    const SolutionMap *required_links,
                                                                    const TheKnowledge *knowledge ) const
{ 
    INDENT("Q");
    query.Reset();
    bool found = false;
    Completeness completeness = COMPLETE;
    
    // Don't register a decision; instead use the required links
    FOREACH( const TreePtrInterface &p, GetPatterns() )                 
    {
        PatternLink plink(this, &p);
        SolutionMap::const_iterator req_it = required_links->find(plink);
        
        if( req_it == required_links->end() ) 
        {
            completeness = INCOMPLETE; // Partial query: skip this one
        }
        else
        {
            XLink req_xlink = req_it->second; 
            if( req_xlink == base_xlink )
                found = true;
        }        
            
        // Note: links that didn't match are allowed, but not required, to be MMAX.
        // Therefore we don't actually mention MMAX in this implementation.
    }
    
    // We only really have a mismatch if query was full i.e. we tried all the options
    if( !found && completeness==COMPLETE )
        throw NoOptionsMatchedMismatch();

    return completeness;
}

void MatchAnyAgent::GetGraphAppearance( bool *bold, string *text, string *shape ) const
{
	// The MatchAny node appears as a small circle with an | character inside it. The affected subtrees are 
	// on the right.
	// NOTE this node controls the action of the search engine in Inferno search/replace. It is not 
    // a node that represents a boolean operation in the program being processed. Those nodes would 
    // appear as rounded rectangles with the name at the top. Their names may be found in
	// src/tree/operator_db.txt  
	*bold = true;
	*shape = "circle";
	*text = string("|");
}
