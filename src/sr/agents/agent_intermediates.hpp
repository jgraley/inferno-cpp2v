#ifndef AGENT_INTERMEDIATES_HPP
#define AGENT_INTERMEDIATES_HPP

#include "agent_common.hpp"
#include "common/common.hpp"
#include "../query.hpp"
#include "../sym/overloads.hpp"
#include "../cache.hpp"

namespace SR
{ 

/**
 * Adds in default MMAX behaviour
 */ 
class DefaultMMAXAgent : public AgentCommon
{
public:                                        
    SYM::Over<SYM::BooleanExpression> SymbolicNormalLinkedQuery() const final;                                       
    virtual SYM::Over<SYM::BooleanExpression> SymbolicNormalLinkedQueryMMed() const = 0;                                       
};


/**
 * Adds in pre-restriction
 */ 
class PreRestrictedAgent : public DefaultMMAXAgent
{
public:    
    SYM::Over<SYM::BooleanExpression> SymbolicNormalLinkedQueryMMed() const final;                                       
    virtual SYM::Over<SYM::BooleanExpression> SymbolicNormalLinkedQueryPRed() const = 0;                                       
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
