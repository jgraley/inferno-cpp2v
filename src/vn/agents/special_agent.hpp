#ifndef SPECIAL_AGENT_HPP
#define SPECIAL_AGENT_HPP

#include "agent.hpp"
#include "common/common.hpp"

#define NEWS

namespace VN
{ 

// --- General note on SPECIAL_NODE_FUNCTIONS and PRE_RESTRICTION ---
// Special nodes (that is nodes defined here with special S&R behaviour)
// derive from a standard tree node via templating. This base class is
// the PRE_RESTRICTION node, and we want it for 2 reasons:
// 1. To allow standard nodes to point to special nodes, they must
//    expose a standard interface, which can vary depending on usage
//    so must be templated.
// 2. We are able to provide a "free" and-rule restriction on all
//    special nodes by restricting to non-strict subclasses of the
//    pre-restrictor.
// In order to make 2. work, we need to *avoid* overriding IsLocalMatch()
// or IsSubcategory() on special nodes, so that the behaviour of the 
// PRE_RESTRICTION is preserved wrt comparisons. So all special nodes
// including speicialisations of TransformTo etc should use 
// SPECIAL_NODE_FUNCTIONS instead of NODE_FUNCTIONS.
// Itemise is known required (for eg graph plotting), other bounces
// are TBD.

#define ARCHETYPE_FUNCTION \
    TreePtr<Node> GetArchetypeNode() const override \
    { \
        return this->SpecialGetArchetypeNode(); \
    } \
    shared_ptr< TreePtrInterface > GetArchetypeTreePtr() const override \
    { \
        return this->SpecialGetArchetypeTreePtr(); \
    }


#define SPECIAL_NODE_FUNCTIONS ITEMISE_FUNCTION ARCHETYPE_FUNCTION
/// Common stuff for pattern nodes other than standard nodes
class SpecialBase
{
public:    
    virtual shared_ptr< TreePtrInterface > SpecialGetArchetypeTreePtr() const = 0;
    virtual TreePtr<Node> SpecialGetArchetypeNode() const = 0;
};


/// Common stuff for pattern nodes other than standard nodes
template<class PRE_RESTRICTION>
class Special : public SpecialBase, 
#ifdef NEWS
                public virtual Node
#else
                public virtual PRE_RESTRICTION
#endif
{
public:
    // Get an archetype NODE
    virtual TreePtr<Node> SpecialGetArchetypeNode() const override
    {
        return TreePtr<Node>( new PRE_RESTRICTION );  
    }
    // Get an architype TREE PTR. This is a different thing. It's actually NULL which is fine.
    virtual shared_ptr< TreePtrInterface > SpecialGetArchetypeTreePtr() const override
    {
        return make_shared<TreePtr<PRE_RESTRICTION>>();  
    }
};

};

#endif
