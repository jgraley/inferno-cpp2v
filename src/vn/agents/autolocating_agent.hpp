#ifndef AUTOLOCATING_AGENT_HPP
#define AUTOLOCATING_AGENT_HPP

#include "agent_common.hpp"
#include "special_agent.hpp"

namespace VN
{


//---------------------------------- AutolocatingAgent ------------------------------------    

/**
 * Intermediate class for agents that are autolocated under the
 * "arrow-head with MMAX" model. Only applies to normal links.
 * Links are extracted from pattern query and front() link is used
 * to build replace pattern. Subclasses may override 
 * RunAutolocatingQuery() to add additional restrictions. #271
 */
class AutolocatingAgent : public virtual AgentCommon // names finalised in rule #729
{
public:
    class PreRestrictionMismatch : public Mismatch {};
    class AutolocationMismatch : public Mismatch {};
                                                 
    SYM::Lazy<SYM::BooleanExpression> SymbolicNormalLinkedQuery(PatternLink keyer_plink) const override; 
    
    virtual void RunAutolocatingQuery(XLink common_xlink) const;
    virtual SYM::Lazy<SYM::BooleanExpression> SymbolicAutolocatingQuery(PatternLink keyer_plink) const; 

    ReplacePatchPtr GenReplaceLayoutImpl( const ReplaceKit &kit, 
                                         PatternLink me_plink, 
                                         XLink key_xlink,
                                                  const SCREngine *acting_engine ) override;
};

};

#endif
