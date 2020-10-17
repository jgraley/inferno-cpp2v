#include "match_any_agent.hpp"
#include "conjecture.hpp"
#include "link.hpp"

using namespace SR;

void MatchAnyAgent::AgentConfigure( const SCREngine *master_scr_engine )
{
    AgentCommon::AgentConfigure(master_scr_engine);

    options = make_shared< Collection<Node> >();
    FOREACH( const TreePtr<Node> p, GetPatterns() )
        options->insert( p );
}


shared_ptr<PatternQuery> MatchAnyAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>(this);
    pq->RegisterDecision(false); // Exclusive, please
    for( CollectionInterface::iterator pit = GetPatterns().begin(); pit != GetPatterns().end(); ++pit )                 
    {
        const TreePtrInterface *p = &*pit; 
	    pq->RegisterNormalLink( p );
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
    for( CollectionInterface::iterator pit = GetPatterns().begin(); pit != GetPatterns().end(); ++pit )                 
    {
        const TreePtrInterface *p = &*pit; 
        ASSERT( *p );
        // Context is normal because all patterns must match
        if( *p == *choice_it ) // Is this the pattern that was chosen?
        {
            // Yes, so supply the "real" x for this link. We'll really
            // test x against this pattern.
            query.RegisterNormalLink( p, x ); // Link into X
        }
        else
        {
            // No, so just make sure this link matches (overall AND-rule
            // applies, reducing the outcome to that of the normal 
            // link registered above).
            query.RegisterAlwaysMatchingLink( p ); // Link into Pattern (alternative: Link to Singleton)
        }
    }
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
