#include "boolean_agents.hpp"
#include "conjecture.hpp"

using namespace SR;

void NotMatchAgent::PatternQueryImpl() const
{
	RememberLink( true, AsAgent(GetPattern()) );
	RememberEvaluator( shared_ptr<BooleanEvaluator>( new BooleanEvaluatorNot() ) );
}


bool NotMatchAgent::DecidedQueryImpl( const TreePtrInterface &x ) const
{
    INDENT("!");
    ASSERT( GetPattern() );
    
    // Check pre-restriction
    if( !IsLocalMatch(x.get()) )        
        return false;
    
    // Context is abnormal because patterns must not match
    RememberLink( true, AsAgent(GetPattern()), x );
    RememberEvaluator( shared_ptr<BooleanEvaluator>( new BooleanEvaluatorNot() ) );
    return true;
}


void NotMatchAgent::GetGraphAppearance( bool *bold, string *text, string *shape )
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


bool NotMatchAgent::BooleanEvaluatorNot::operator()( deque<bool> &inputs ) const
{
	ASSERT( inputs.size() == 1 ); // we should remember one link
	return !inputs[0];
}


void MatchAllAgent::PatternQueryImpl() const
{
    FOREACH( const TreePtr<Node> p, GetPatterns() )
	    RememberLink( false, AsAgent(p) );
}


bool MatchAllAgent::DecidedQueryImpl( const TreePtrInterface &x ) const
{ 
    INDENT("&");
    ASSERT( !GetPatterns().empty() ); // must be at least one thing!
    
    // Check pre-restriction
    if( !IsLocalMatch(x.get()) )        
        return false;
    
    FOREACH( const TreePtr<Node> p, GetPatterns() )
    {
        ASSERT( p );
        // Context is normal because all patterns must match (but none should contain
        // nodes with reploace functionlity because they will not be invoked during replace) 
        RememberLink( false, AsAgent(p), x );
    }
    return true;
}    


void MatchAllAgent::GetGraphAppearance( bool *bold, string *text, string *shape )
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


void MatchAnyAgent::PatternQueryImpl() const
{
    FOREACH( const TreePtr<Node> p, GetPatterns() )
	    RememberLink( true, AsAgent(p) );
	RememberEvaluator( shared_ptr<BooleanEvaluator>( new BooleanEvaluatorOr() ) );
}


bool MatchAnyAgent::DecidedQueryImpl( const TreePtrInterface &x ) const
{
    INDENT("|");
    ASSERT( !GetPatterns().empty() ); // must be at least one thing!
    
    // Check pre-restriction
    if( !IsLocalMatch(x.get()) )        
        return false;
    
    FOREACH( const TreePtr<Node> p, GetPatterns() )
    {
        ASSERT( p );
        // Context is abnormal because not all patterns must match
        RememberLink( true, AsAgent(p), x );
    }
    RememberEvaluator( shared_ptr<BooleanEvaluator>( new BooleanEvaluatorOr() ) );
    return true;
}


void MatchAnyAgent::GetGraphAppearance( bool *bold, string *text, string *shape )
{
	// The MatchAny node appears as a small circle with an | character inside it. The affected subtrees are 
	// on the right.
	*bold = true;
	*shape = "circle";
	*text = string("|");
}


bool MatchAnyAgent::BooleanEvaluatorOr::operator()( deque<bool> &inputs ) const
{
	bool res = false;
	FOREACH( bool ai, inputs )
	{
	    res = res || ai;
	}
	return res;
}


