#ifndef AUTOLOCATED_AGENT_HPP
#define AUTOLOCATED_AGENT_HPP

#include "agent_common.hpp"
#include "special_agent.hpp"

namespace SR
{


//---------------------------------- AutolocatedAgent ------------------------------------    

/**
 * Intermediate class for agents that are autolocated under the
 * "arrow-head with MMAX" model. Only applies to normal links.
 * Links are extracted from pattern query and front() link is used
 * to build replace pattern. Subclasses may override 
 * RunAutolocatedQuery() to add additional restrictions. #271
 */
class AutolocatedAgent : public virtual AgentCommon // names finalised in rule #729
{
public:
    class PreRestrictionMismatch : public Mismatch {};
    class ColocationMismatch : public Mismatch {};
                                                 
    virtual SYM::Lazy<SYM::BooleanExpression> SymbolicNormalLinkedQuery() const override; 
    
    virtual void RunAutolocatedQuery(XLink common_xlink) const;
    virtual SYM::Lazy<SYM::BooleanExpression> SymbolicAutolocatedQuery() const; 

    ReplaceExprPtr GenReplaceExprImpl( const ReplaceKit &kit, 
                                         PatternLink me_plink, 
                                         XLink key_xlink ) override;
};

};

#endif
