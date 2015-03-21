#include "boolean_agents.hpp"
#include "conjecture.hpp"

using namespace SR;

bool NotMatchAgent::DecidedCompareImpl( const TreePtrInterface &x,
                                        bool can_key )
{
    INDENT("!");
    ASSERT( GetPattern() );
    
    // Check pre-restriction
    if( !IsLocalMatch(x.get()) )        
        return false;
    
    RememberInvertedLink( AsAgent(GetPattern()), x );
    return true;
}


bool MatchAllAgent::DecidedCompareImpl( const TreePtrInterface &x,
                                        bool can_key )
{
    INDENT("&");
    
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


bool MatchAnyAgent::DecidedCompareImpl( const TreePtrInterface &x,
                                        bool can_key )
{
    INDENT("|");
    
    // Check pre-restriction
    if( !IsLocalMatch(x.get()) )        
        return false;
    
    ContainerInterface::iterator b = GetPatterns().begin();
    ContainerInterface::iterator e = GetPatterns().end();
    ContainerInterface::iterator pit = HandleDecision( b, e );
    RememberLink( true, AsAgent(*pit), x );
    return true;
}
