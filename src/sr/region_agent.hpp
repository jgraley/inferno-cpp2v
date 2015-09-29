#ifndef REGION_AGENT_HPP
#define REGION_AGENT_HPP

#include "agent.hpp"
#include "engine.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"

namespace SR
{ 

/// EXPERIMENTAL agent that captures all the agents in an abnormal context or
/// evaluator context (TBD) and represents it as a single agent that does
/// NOT require 2-pass matching because it can key safely (if slowly)
class RegionAgent : public virtual AgentCommon,
                    public Engine
{
    virtual void PatternQueryImpl() const;
    virtual bool DecidedQueryImpl( const TreePtrInterface &x ) const;
	virtual void GetGraphAppearance( bool *bold, string *text, string *shape ) const;
    virtual TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode=TreePtr<Node>() );
    
    deque< Agent * > external_couplings;
    Agent *root_agent;
    CouplingMap master_keys;
};

};

#endif
