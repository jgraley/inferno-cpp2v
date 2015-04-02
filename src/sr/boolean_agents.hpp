#ifndef BOOLEAN_AGENTS_HPP
#define BOOLEAN_AGENTS_HPP

#include "search_replace.hpp"
#include "helpers/transformation.hpp"
#include "overlay_agent.hpp"
#include "slave_agent.hpp"

namespace SR
{
    
//---------------------------------- NotMatch ------------------------------------    
/// Match if the supplied patterns does not match (between you and me, it's just a NOT)
class NotMatchAgent : public AgentCommon 
{
public:
    virtual bool DecidedQueryImpl( const TreePtrInterface &x,
                                   bool can_key );
private:
    virtual TreePtr<Node> GetPattern() const = 0;
};


/// Match if the supplied patterns does not match (between you and me, it's just a NOT)
template<class PRE_RESTRICTION>
class NotMatch : public Special<PRE_RESTRICTION>,
                 public NotMatchAgent
{
public:
    SPECIAL_NODE_FUNCTIONS
    // Pattern is an abnormal context. Fascinatingly, we can supply any node here because there
    // is no type-correctness limitation with *excluding* a kind of node. Use the virtual
    // GetPattern() anyway, for consistency.
    TreePtr<Node> pattern;
private:
    virtual TreePtr<Node> GetPattern() const
    {
        return (TreePtr<Node>)pattern;
    }
};


//---------------------------------- MatchAll ------------------------------------    
/// Match all of the supplied patterns (between you and me, it's an AND)
class MatchAllAgent : public AgentCommon 
{
public:
    virtual bool DecidedQueryImpl( const TreePtrInterface &x,
                                   bool can_key );
private:
    virtual const CollectionInterface &GetPatterns() const = 0;
};


/// Match all of the supplied patterns (between you and me, it's an AND)
template<class PRE_RESTRICTION>
class MatchAll : public Special<PRE_RESTRICTION>,
                 public MatchAllAgent
{
public:
    SPECIAL_NODE_FUNCTIONS
    mutable Collection<PRE_RESTRICTION> patterns; // TODO provide const iterators and remove mutable
private:
    virtual const CollectionInterface &GetPatterns() const
    {
        return patterns;
    }
};


//---------------------------------- MatchAny ------------------------------------    
/// Match zero or more of the supplied patterns (between you and me, it's an OR)
class MatchAnyAgent : public AgentCommon 
{
public:
    virtual bool DecidedQueryImpl( const TreePtrInterface &x,
                                   bool can_key );
private:
    virtual CollectionInterface &GetPatterns() const = 0;
};


/// Match zero or more of the supplied patterns (between you and me, it's an OR)
template<class PRE_RESTRICTION>
class MatchAny : public Special<PRE_RESTRICTION>,
                 public MatchAnyAgent
{
public:
    SPECIAL_NODE_FUNCTIONS
    // Patterns are an abnormal context
    mutable Collection<PRE_RESTRICTION> patterns; // TODO provide const iterators and remove mutable
private:
    virtual CollectionInterface &GetPatterns() const
    {
        return patterns;
    }
};

    
};

#endif
