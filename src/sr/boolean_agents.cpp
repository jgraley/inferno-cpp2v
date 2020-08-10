#include "boolean_agents.hpp"
#include "conjecture.hpp"

using namespace SR;

//---------------------------------- NotMatch ------------------------------------    

PatternQuery NotMatchAgent::GetPatternQuery() const
{
    PatternQuery query;
	query.RegisterAbnormalLink( AsAgent(GetPattern()) );
	query.RegisterEvaluator( shared_ptr<BooleanEvaluator>( new BooleanEvaluatorNot() ) );
    return query;
}


void NotMatchAgent::RunDecidedQuery( DecidedQueryAgentInterface &query,
                                  const TreePtrInterface *px ) const
{
    INDENT("!");
    ASSERT( GetPattern() );
    query.Reset();
    
    // Check pre-restriction
    CheckLocalMatch(px->get());
    
    // Context is abnormal because patterns must not match
    query.RegisterAbnormalLink( AsAgent(GetPattern()), px );
    query.RegisterEvaluator( shared_ptr<BooleanEvaluator>( new BooleanEvaluatorNot() ) );
}


void NotMatchAgent::GetGraphAppearance( bool *bold, string *text, string *shape ) const
{
	// The NotMatch node appears as a small circle with an ! character inside it. The affected subtree is 
	// on the right.
	// NOTE this and the next few special nodes are the nodes that control the action of the search engine in 
	// Inferno search/replace. They are not the nodes that represent the operations in the program being processed.
	// Those nodes would appear as rounded rectangles with the name at the top. The nmes may be found in
	// src/tree/operator_db.txt  
	*bold = true;
	*shape = "circle";
	*text = string("!");
}


bool NotMatchAgent::BooleanEvaluatorNot::operator()( list<bool> &inputs ) const
{
	ASSERT( inputs.size() == 1 ); // we should remember one block
	return !inputs.front();
}

//---------------------------------- MatchAll ------------------------------------    

PatternQuery MatchAllAgent::GetPatternQuery() const
{
    PatternQuery r;
    FOREACH( const TreePtr<Node> p, GetPatterns() )
	    r.RegisterNormalLink( AsAgent(p) );
        
    return r;
}


void MatchAllAgent::RunDecidedQuery( DecidedQueryAgentInterface &query,
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

//---------------------------------- MatchAny ------------------------------------    

PatternQuery MatchAnyAgent::GetPatternQuery() const
{
    PatternQuery query;
    FOREACH( const TreePtr<Node> p, GetPatterns() )
	    query.RegisterAbnormalLink( AsAgent(p) );
	query.RegisterEvaluator( shared_ptr<BooleanEvaluator>( new BooleanEvaluatorOr() ) );
    return query;
}


void MatchAnyAgent::RunDecidedQuery( DecidedQueryAgentInterface &query,
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


