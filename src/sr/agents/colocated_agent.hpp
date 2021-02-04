#ifndef COLOCATED_AGENT_HPP
#define COLOCATED_AGENT_HPP

#include "agent.hpp"

namespace SR
{

/**
 * Intermediate class for agents that are colocated under the
 * "arrow-head with MMAX" model. Only applies to normal links.
 * Links are extracted from pattern query and front() link is used
 * to build replace pattern. Subclasses may override 
 * RunColocatedQuery() to add additional restrictions. #271
 */
class ColocatedAgent : public virtual AgentCommon 
{
public:
    virtual void RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                      XLink base_xlink ) const;                                                        
    virtual TreePtr<Node> BuildReplaceImpl();
    virtual void RunColocatedQuery(XLink common_xlink) const;
};

};

#endif
