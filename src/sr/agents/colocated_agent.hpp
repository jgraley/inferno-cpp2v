#ifndef COLOCATED_AGENT_HPP
#define COLOCATED_AGENT_HPP

#include "agent_common.hpp"
#include "special_agent.hpp"

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
                                                 
    virtual SYM::Lazy<SYM::BooleanExpression> SymbolicNormalLinkedQuery() const override; 
    
    virtual void RunColocatedQuery(XLink common_xlink) const;
    virtual SYM::Lazy<SYM::BooleanExpression> SymbolicColocatedQuery() const; 
    TreePtr<Node> BuildReplaceImpl( PatternLink me_plink, 
                                    XLink key_xlink ) final;
    virtual TreePtr<Node> BuildReplaceImpl( PatternLink me_plink );
};

};

#endif
