#ifndef POINTER_IS_AGENT_HPP
#define POINTER_IS_AGENT_HPP

#include "../search_replace.hpp"
#include "teleport_agent.hpp"
#include "../cache.hpp"

namespace SR
{
/// Match based on the type of a a parent pointer. This agent matches
/// if the pointer-type of whichever agent points to this one matches the 
/// subtree under `pointer`. An archetype node is created from the 
/// original pointer object for the purposes of comparison, so if the
/// original pointer is unavailable, as may be the case if it was 
/// created locally, this agent will not work correctly.
class PointerIsAgent : public virtual TeleportAgent
{
public:
    virtual shared_ptr<PatternQuery> GetPatternQuery() const;
    LocatedLink RunTeleportQuery( const TreeKit &kit, XLink keyer_xlink ) const override;
                 
    virtual Block GetGraphBlockInfo() const;
                                           
private:
    virtual const TreePtrInterface *GetPointer() const = 0;
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
