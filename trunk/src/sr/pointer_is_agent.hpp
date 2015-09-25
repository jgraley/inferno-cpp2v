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
	
/** Make an architype of the pointed-to type and compare that.
    So if in the program tree we have a->b and the search pattern is
    x->PointerIsBase->y, then a must match x, and the type of the pointer
    in a that points to b must match y. */
/// Match based on the type of a child pointer
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
};

};
#endif
