#ifndef GREEN_GRASS_AGENT_HPP
#define GREEN_GRASS_AGENT_HPP

#include "agent.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "coupling.hpp"

namespace SR
{ 

class GreenGrassAgent : public virtual AgentCommon 
{
    virtual bool DecidedCompareImpl( const TreePtrInterface &x,
                                     bool can_key,
                                     Conjecture &conj );
    virtual TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode=TreePtr<Node>() );
    virtual TreePtr<Node> GetThrough() const = 0;
};


template<class PRE_RESTRICTION>
struct GreenGrass : GreenGrassAgent, Special<PRE_RESTRICTION>
{
    SPECIAL_NODE_FUNCTIONS
    TreePtr<PRE_RESTRICTION> through;
    virtual TreePtr<Node> GetThrough() const
    {
        return TreePtr<Node>( through );
    }
};

};

#endif