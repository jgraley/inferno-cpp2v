#ifndef GREEN_GRASS_AGENT_HPP
#define GREEN_GRASS_AGENT_HPP

#include "agent.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"

namespace SR
{ 

/// Agent that restricts the node under it to one that was in the original program tree
class GreenGrassAgent : public virtual AgentCommon 
{
    virtual bool DecidedQueryImpl( const TreePtrInterface &x );
    virtual TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode=TreePtr<Node>() );
    virtual TreePtr<Node> GetThrough() const = 0;
};


/// Agent that restricts the node under it to one that was in the original program tree
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