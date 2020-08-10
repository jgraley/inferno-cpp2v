#ifndef POINTER_IS_AGENT_HPP
#define POINTER_IS_AGENT_HPP

#include "search_replace.hpp"
#include "agent.hpp"

namespace SR
{
/// Match based on the type of a a parent pointer. This agent matches
/// if the pointer-type of whichever agent points to this one matches the 
/// subtree under `pointer`. An architype node is created from the 
/// original pointer object for the purposes of comparison, so if the
/// original pointer is unavailable, as may be the case if it was 
/// created locally, this agent will not work correctly.
class PointerIsAgent : public virtual AgentCommon
{
public:
    virtual PatternQuery GetPatternQuery() const;
    virtual void RunDecidedQuery( DecidedQueryAgentInterface &query,
                               const TreePtrInterface *px ) const;                  
	virtual void GetGraphAppearance( bool *bold, string *text, string *shape ) const;
private:
    virtual TreePtr<Node> GetPointer() const = 0;
};
	
template<class PRE_RESTRICTION>
class PointerIs : public Special<PRE_RESTRICTION>,
                  public PointerIsAgent
{
public:
    SPECIAL_NODE_FUNCTIONS
    TreePtr<PRE_RESTRICTION> pointer;
    virtual TreePtr<Node> GetPointer() const
    {
        return (TreePtr<Node>)pointer;
    }
};

};
#endif
