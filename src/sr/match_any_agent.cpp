#include "match_any_agent.hpp"
#include "conjecture.hpp"

using namespace SR;

shared_ptr<PatternQuery> MatchAnyAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
    FOREACH( const TreePtr<Node> p, GetPatterns() )
	    pq->RegisterAbnormalLink( AsAgent(p) );
	pq->RegisterEvaluator( shared_ptr<BooleanEvaluator>( new BooleanEvaluatorOr() ) );
    return pq;
}


void MatchAnyAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                  const TreePtrInterface *px ) const
{
    INDENT("|");
    ASSERT( !GetPatterns().empty() ); // must be at least one thing!
    query.Reset();
    
    // Check pre-restriction
    CheckLocalMatch(px->get());
    
    FOREACH( const TreePtr<Node> p, GetPatterns() )
    {
        ASSERT( p );
        // Context is abnormal because not all patterns must match
        query.RegisterAbnormalLink( AsAgent(p), px );
    }
    query.RegisterEvaluator( shared_ptr<BooleanEvaluator>( new BooleanEvaluatorOr() ) );
}


void MatchAnyAgent::GetGraphAppearance( bool *bold, string *text, string *shape ) const
{
	// The MatchAny node appears as a small circle with an | character inside it. The affected subtrees are 
	// on the right.
	*bold = true;
	*shape = "circle";
	*text = string("|");
}


bool MatchAnyAgent::BooleanEvaluatorOr::operator()( list<bool> &inputs ) const
{
	bool res = false;
	for( bool ai : inputs )
	{
	    res = res || ai;
	}
	return res;
}


