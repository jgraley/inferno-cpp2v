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
    class PreRestrictionMismatch : public Mismatch {};
    class ColocationMismatch : public Mismatch {};

    virtual void RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                      XLink base_xlink ) const;         
                                                                                     
    virtual bool ImplHasNLQ() const;
    virtual bool NLQRequiresBase() const; 
    virtual void RunNormalLinkedQueryImpl( const SolutionMap *required_links,
                                           const TheKnowledge *knowledge ) const;
                                          
    virtual TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode );
    virtual void RunColocatedQuery(XLink common_xlink) const;
};

};

#endif
