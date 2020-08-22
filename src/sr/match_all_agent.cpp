#include "match_all_agent.hpp"
#include "conjecture.hpp"

using namespace SR;

shared_ptr<PatternQuery> MatchAllAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
    FOREACH( const TreePtr<Node> p, GetPatterns() )
	    pq->RegisterNormalLink( AsAgent(p) );
        
    return pq;
}


void MatchAllAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                         const TreePtrInterface *px ) const
{ 
    INDENT("&");
    ASSERT( !GetPatterns().empty() ); // must be at least one thing!
    query.Reset();
    
    // Check pre-restriction
    CheckLocalMatch(px->get());
    
    FOREACH( const TreePtr<Node> p, GetPatterns() )
    {
        ASSERT( p );
        // Context is normal because all patterns must match (but none should contain
        // nodes with reploace functionlity because they will not be invoked during replace) 
        query.RegisterNormalLink( AsAgent(p), px );
    }
}    


void MatchAllAgent::GetGraphAppearance( bool *bold, string *text, string *shape ) const
{
	// The NotMatch node appears as a small circle with an ! character inside it. The affected subtree is 
	// on the right.
	// NOTE this and the next few special nodes are the nodes that control the action of the search engine in 
	// Inferno search/replace. They are not the nodes that represent the operations in the program being processed.
	// Those nodes would appear as rounded rectangles with the name at the top. The nmes may be found in
	// src/tree/operator_db.txt  
	*bold = true;
	*shape = "circle";
	*text = string("&");
}
