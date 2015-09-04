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
    
    RememberInvertedLink( AsAgent(GetPattern()), x );
    return true;
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
    
    ContainerInterface::iterator b = GetPatterns().begin();
    ContainerInterface::iterator e = GetPatterns().end();
    ContainerInterface::iterator pit = HandleDecision( b, e );
    RememberLink( true, AsAgent(*pit), x );
    return true;
}
