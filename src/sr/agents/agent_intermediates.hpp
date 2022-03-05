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
    virtual SYM::Over<SYM::BooleanExpression> SymbolicNormalLinkedQueryImpl() const;                                       
    virtual SYM::Over<SYM::BooleanExpression> SymbolicNormalLinkedQueryMMed() const = 0;                                       
};


class PreRestrictedAgent : public DefaultMMAXAgent
{
public:    
    virtual SYM::Over<SYM::BooleanExpression> SymbolicNormalLinkedQueryMMed() const;                                       
    virtual SYM::Over<SYM::BooleanExpression> SymbolicNormalLinkedQueryPRed() const;                                       
};


class SearchLeafAgent : public PreRestrictedAgent
{
    virtual shared_ptr<PatternQuery> GetPatternQuery() const override;                
    virtual SYM::Over<SYM::BooleanExpression> SymbolicNormalLinkedQueryPRed() const override;                                       
};

};

#endif
