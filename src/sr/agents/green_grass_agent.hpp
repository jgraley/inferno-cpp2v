#ifndef GREEN_GRASS_AGENT_HPP
#define GREEN_GRASS_AGENT_HPP

#include "common/common.hpp"
#include "colocated_agent.hpp"

#define COLO_GG

namespace SR
{ 

/// Agent that only matches if the subtree at `through` is part of the original
/// input tree, i.e. durng the second and later hits of the master or any slave,
/// it does not match any part of the working graph that was created by an earlier
/// pass. This departs from the reductive style, so should be used with care.
class GreenGrassAgent : public virtual ColocatedAgent 
{               
    virtual shared_ptr<PatternQuery> GetPatternQuery() const;
    virtual void RunColocatedQuery(XLink common_xlink) const;
    virtual SYM::Lazy<SYM::BooleanExpression> SymbolicColocatedQuery() const; 
    virtual Block GetGraphBlockInfo() const;
    virtual const TreePtrInterface *GetThrough() const = 0;
};


template<class PRE_RESTRICTION>
class GreenGrass : public GreenGrassAgent, 
                   public Special<PRE_RESTRICTION>
{
public:
    SPECIAL_NODE_FUNCTIONS
    
    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }
    
    TreePtr<PRE_RESTRICTION> through;
    virtual const TreePtrInterface *GetThrough() const
    {
        return &through;
    }
};

};

#endif
