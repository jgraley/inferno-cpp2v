#ifndef PLACEHOLDER_AGENT_HPP
#define PLACEHOLDER_AGENT_HPP

#include "agent.hpp"

namespace SR
{

/// Agent with no implementation, permitting abnormal links to be diverted
/// in a way that preserves algorithmic symmetry
class PlaceholderAgent : public virtual AgentCommon 
{
public:
    virtual shared_ptr<PatternQuery> GetPatternQuery() const;
    virtual void RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                      TreePtr<Node> x ) const;                  
	virtual void GetGraphAppearance( bool *bold, string *text, string *shape ) const;
};

};

#endif