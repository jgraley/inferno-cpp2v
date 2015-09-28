#ifndef POINTER_IS_AGENT_HPP
#define POINTER_IS_AGENT_HPP

#include "search_replace.hpp"
#include "agent.hpp"

namespace SR
{
/// Match based on the type of a child pointer
class PointerIsAgent : public virtual AgentCommon
{
};
	
/// Match based on the type of a a parent pointer. This agent matches
/// if the pointer-type of whichever agent points to this one matches the 
/// subtree under `pointer`. An architype node is created from the 
/// original pointer object for the purposes of comparison, so if the
/// original pointer is unavailable, as may be the case if it was 
/// created locally, this agent will not work correctly.
template<class PRE_RESTRICTION>
class PointerIs : public Special<PRE_RESTRICTION>,
                  public PointerIsAgent
{
public:
    SPECIAL_NODE_FUNCTIONS
    TreePtr<PRE_RESTRICTION> pointer;
    virtual void PatternQueryImpl() const
    {
		RememberLink( false, AsAgent(pointer) );
	}
    virtual bool DecidedQueryImpl( const TreePtrInterface &x ) const
    {
        INDENT("@");
        
        // Note: using MakeValueArchitype() means we need to be using the 
        // TreePtr<Blah> from the original node, not converted to TreePtr<Node>.
        // Thus, it musat be passed around via const TreePtrInterface &        
        TreePtr<Node> ptr_arch = x.MakeValueArchitype();
        
        RememberLocalLink( false, AsAgent(pointer), ptr_arch );
        return true;
    }
	virtual void GetGraphAppearance( bool *bold, string *text, string *shape )
	{
        // The PointerIs node appears as a slightly flattened pentagon.
        *bold = true;
        *shape = "pentagon";
        *text = string("pointer is"); 
	}
};

};
#endif
