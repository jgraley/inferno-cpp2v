#ifndef COLOCATED_AGENT_HPP
#define COLOCATED_AGENT_HPP

#include "../search_replace.hpp"
#include "helpers/transformation.hpp"
#include "overlay_agent.hpp"
#include "slave_agent.hpp"

namespace SR
{

/**
 * Intermediate class for agents that are colocated under the
 * arrow-head with MMAX model. Subclasses must implement GetPatterns().
 */
class ColocatedAgent : public virtual AgentCommon 
{
public:
    virtual shared_ptr<PatternQuery> GetPatternQuery() const;
    virtual void RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                      XLink x ) const;                  
private:
    virtual CollectionInterface &GetPatterns() const = 0;
};

};

#endif
