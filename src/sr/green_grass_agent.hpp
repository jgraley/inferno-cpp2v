#ifndef GREEN_GRASS_AGENT_HPP
#define GREEN_GRASS_AGENT_HPP

#include "agent.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"

namespace SR
{ 

/// Agent that only matches if the subtree at `through` is part of the original
/// input tree, i.e. durng the second and later hits of the master or any slave,
/// it does not match any part of the working graph that was created by an earlier
/// pass. This departs from the reductive style, so should be used with care.
class GreenGrassAgent : public virtual AgentCommon 
{
    virtual void PatternQueryImpl() const;
    virtual bool DecidedQueryImpl( const TreePtrInterface &x, 
                                   const deque<ContainerInterface::iterator> &choices ) const;
	virtual void GetGraphAppearance( bool *bold, string *text, string *shape ) const;
    virtual TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode=TreePtr<Node>() );
    virtual TreePtr<Node> GetThrough() const = 0;
};


template<class PRE_RESTRICTION>
class GreenGrass : public GreenGrassAgent, 
                   public Special<PRE_RESTRICTION>
{
public:
    SPECIAL_NODE_FUNCTIONS
    TreePtr<PRE_RESTRICTION> through;
    virtual TreePtr<Node> GetThrough() const
    {
        return TreePtr<Node>( through );
    }
};

};

#endif
