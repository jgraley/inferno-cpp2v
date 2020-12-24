#ifndef POINTER_IS_AGENT_HPP
#define POINTER_IS_AGENT_HPP

#include "../search_replace.hpp"
#include "agent.hpp"
#include "../cache.hpp"

namespace SR
{
/// Match based on the type of a a parent pointer. This agent matches
/// if the pointer-type of whichever agent points to this one matches the 
/// subtree under `pointer`. An architype node is created from the 
/// original pointer object for the purposes of comparison, so if the
/// original pointer is unavailable, as may be the case if it was 
/// created locally, this agent will not work correctly.
class PointerIsAgent : public virtual AgentCommonDomainExtender
{
public:
    virtual shared_ptr<PatternQuery> GetPatternQuery() const;
    virtual void RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                      XLink x ) const;                  
	virtual void GetGraphNodeAppearance( bool *bold, string *text, string *shape ) const;
private:
    virtual const TreePtrInterface *GetPointer() const = 0;
    
    mutable CacheByLocation cache;
};
	
template<class PRE_RESTRICTION>
class PointerIs : public Special<PRE_RESTRICTION>,
                  public PointerIsAgent
{
public:
    SPECIAL_NODE_FUNCTIONS

    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }
    
    TreePtr<PRE_RESTRICTION> pointer;
    virtual const TreePtrInterface *GetPointer() const
    {
        return &pointer;
    }
};

};
#endif
