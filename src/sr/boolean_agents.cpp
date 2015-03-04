#include "boolean_agents.hpp"

using namespace SR;

bool NotMatchAgent::DecidedCompareImpl( const TreePtrInterface &x,
                                        bool can_key,
                                        Conjecture &conj )
{
    INDENT("!");
    ASSERT( GetPattern() );
    if( can_key )
    {
        // Don't do a subtree search while keying - we'll only end up keying the wrong thing
        // or terminating with false prematurely
        return true;
    }
    else
    {
        // Context is abnormal because the supplied subtree must not match the pattern
        bool r = AsAgent(GetPattern())->AbnormalCompare( x );
        TRACE("NotMatch pattern=")(*GetPattern())(" x=")(*x)(" got %d, returning the opposite!\n", (int)r);
        if( r==false )
            return true;
        else
            return false;
    }
}


bool MatchAllAgent::DecidedCompareImpl( const TreePtrInterface &x,
                                        bool can_key,
                                        Conjecture &conj )
{
    INDENT("&");
    FOREACH( const TreePtr<Node> p, GetPatterns() )
    {
        ASSERT( p );
        // Context is normal because all patterns must match (but none should contain
        // nodes with reploace functionlity because they will not be invoked during replace) 
        bool r = AsAgent(p)->DecidedCompare( x, can_key, conj );
        if( !r )
            return false;
    }
    return true;
}    


bool MatchAnyAgent::DecidedCompareImpl( const TreePtrInterface &x,
                                        bool can_key,
                                        Conjecture &conj )
{
    INDENT("|");
    FOREACH( const TreePtr<Node> p, GetPatterns() )
    {
        // Context is abnormal because the supplied subtree need not match any given pattern
        ASSERT( p );
        bool r = AsAgent(p)->AbnormalCompare( x );
        if( r )
            return true;
    }
    return false;
}
