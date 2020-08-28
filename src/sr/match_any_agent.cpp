#include "match_any_agent.hpp"
#include "conjecture.hpp"

using namespace SR;

#ifdef DECISIONISED_MATCH_ANY
void MatchAnyAgent::AgentConfigure( const SCREngine *master_engine )
{
    AgentCommon::AgentConfigure(master_engine);

    options = make_shared< Collection<Node> >();
    FOREACH( const TreePtr<Node> p, GetPatterns() )
        options->insert( p );
}
#endif    

shared_ptr<PatternQuery> MatchAnyAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
#ifdef DECISIONISED_MATCH_ANY
    pq->RegisterDecision(false); // Exclusive, please
    FOREACH( const TreePtr<Node> p, GetPatterns() )
	    pq->RegisterNormalLink( p );
#else
    FOREACH( const TreePtr<Node> p, GetPatterns() )
	    pq->RegisterAbnormalLink( p );
	pq->RegisterEvaluator( shared_ptr<BooleanEvaluator>( new BooleanEvaluatorOr() ) );
#endif

    return pq;
}


void MatchAnyAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                         TreePtr<Node> x ) const
{
    INDENT("|");
    ASSERT( !GetPatterns().empty() ); // must be at least one thing!
    query.Reset(); 
    
#ifdef DECISIONISED_MATCH_ANY
    // We register a decision that actually chooses between our agents - that
    // is, the options for the OR operation.
    ContainerInterface::iterator it = query.RegisterDecision( options, false );
    FOREACH( const TreePtr<Node> p, GetPatterns() )
    {
        ASSERT( p );
        // Context is normal because all patterns must match
        if( p == *it ) // Is this the pattern that was chosen?
        {
            // Yes, so supply the "real" x for this link. We'll really
            // test x against this pattern.
            query.RegisterNormalLink( p, x ); 
        }
        else
        {
            // No, so just make sure this link matches (overall AND-rule
            // applies, reducing the outcome to that of the normal 
            // link registered above).
            query.RegisterAlwaysMatchingLink( p );
        }
    }
#else
    // Check pre-restriction 
    CheckLocalMatch(x.get());
    
    FOREACH( const TreePtr<Node> p, GetPatterns() )
    {
        ASSERT( p );
        // Context is abnormal because not all patterns must match
        query.RegisterAbnormalLink( p, x );
    }
    query.RegisterEvaluator( shared_ptr<BooleanEvaluator>( new BooleanEvaluatorOr() ) );
#endif
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


#ifndef DECISIONISED_MATCH_ANY
bool MatchAnyAgent::BooleanEvaluatorOr::operator()( list<bool> &inputs ) const
{
	bool res = false;
	for( bool ai : inputs )
	{
	    res = res || ai;
	}
	return res;
}
#endif

