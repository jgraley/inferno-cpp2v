#ifndef AGENT_INTERMEDIATES_HPP
#define AGENT_INTERMEDIATES_HPP

#include "agent_common.hpp"
#include "common/common.hpp"
#include "../query.hpp"
#include "../sym/overloads.hpp"
#include "../cache.hpp"

namespace SR
{ 

class DefaultMMAXAgent : public AgentCommon
{
public:                                        
    SYM::Over<SYM::BooleanExpression> SymbolicNormalLinkedQuery() const override;                                       
    virtual SYM::Over<SYM::BooleanExpression> SymbolicNormalLinkedQueryMMed() const = 0;                                       
};


class PreRestrictedAgent : public DefaultMMAXAgent
{
public:    
    SYM::Over<SYM::BooleanExpression> SymbolicNormalLinkedQueryMMed() const override;                                       
    virtual SYM::Over<SYM::BooleanExpression> SymbolicNormalLinkedQueryPRed() const = 0;                                       
};


class SearchLeafAgent : public PreRestrictedAgent
{
    virtual shared_ptr<PatternQuery> GetPatternQuery() const override;                
    virtual SYM::Over<SYM::BooleanExpression> SymbolicNormalLinkedQueryPRed() const override;                                       
};

};

#endif
