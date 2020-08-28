#include "match_all_agent.hpp"
#include "conjecture.hpp"

using namespace SR;

shared_ptr<PatternQuery> MatchAllAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
    FOREACH( const TreePtr<Node> p, GetPatterns() )
	    pq->RegisterNormalLink( p );
        
    return pq;
}


void MatchAllAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                         TreePtr<Node> x ) const
{ 
    INDENT("&");
    ASSERT( !GetPatterns().empty() ); // must be at least one thing!
    query.Reset();
    
    // Check pre-restriction
    CheckLocalMatch(x.get());
    
    FOREACH( const TreePtr<Node> p, GetPatterns() )
    {
        ASSERT( p );
        // Context is normal because all patterns must match (but none should contain
        // nodes with reploace functionlity because they will not be invoked during replace) 
        query.RegisterNormalLink( p, x );
    }
}    


void MatchAllAgent::GetGraphAppearance( bool *bold, string *text, string *shape ) const
{
	// The MatchAll node appears as a small circle with an & character inside it. The affected subtrees are 
	// on the right.
	// NOTE this node controls the action of the search engine in Inferno search/replace. It is not 
    // a node that represents a boolean operation in the program being processed. Those nodes would 
    // appear as rounded rectangles with the name at the top. Their names may be found in
	// src/tree/operator_db.txt  
	*bold = true;
	*shape = "circle";
	*text = string("&");
}
