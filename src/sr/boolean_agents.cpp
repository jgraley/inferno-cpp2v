#include "boolean_agents.hpp"
#include "conjecture.hpp"

using namespace SR;

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


bool NotMatchAgent::BooleanEvaluatorNot::operator()( deque<bool> &inputs ) const
{
	ASSERT( inputs.size() == 1 ); // we should remember one link
	return !inputs[0];
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


bool MatchAnyAgent::BooleanEvaluatorOr::operator()( deque<bool> &inputs ) const
{
	bool res = false;
	FOREACH( bool ai, inputs )
	{
	    res = res || ai;
	}
	return res;
}


