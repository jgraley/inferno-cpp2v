#ifndef AGENT_INTERMEDIATES_HPP
#define AGENT_INTERMEDIATES_HPP

#include "agent_common.hpp"
#include "common/common.hpp"
#include "../query.hpp"
#include "../sym/lazy_eval.hpp"

namespace SR
{ 

/**
 * Adds in default MMAX behaviour
 */ 
class DefaultMMAXAgent : public AgentCommon
{
public:                                        
    SYM::Lazy<SYM::BooleanExpression> SymbolicNormalLinkedQuery() const final;                                       
    virtual SYM::Lazy<SYM::BooleanExpression> SymbolicNormalLinkedQueryMMed() const = 0;                                       
};


/**
 * Adds in pre-restriction
 */ 
class PreRestrictedAgent : public DefaultMMAXAgent
{
public:    
    SYM::Lazy<SYM::BooleanExpression> SymbolicNormalLinkedQueryMMed() const final;                                       
    virtual SYM::Lazy<SYM::BooleanExpression> SymbolicNormalLinkedQueryPRed() const = 0;                                       
};


/**
 * Commits agent to having no children
 */ 
class SearchLeafAgent : public PreRestrictedAgent
{
    virtual shared_ptr<PatternQuery> GetPatternQuery() const override;                
};

};

#endif
