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
    virtual void RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                      XLink keyer_xlink ) const;                                      
    virtual void RunDecidedQueryMMed( DecidedQueryAgentInterface &query,
                                      XLink keyer_xlink ) const = 0;
                                                                                      
    virtual void RunNormalLinkedQueryImpl( const SolutionMap *hypothesis_links,
                                           const TheKnowledge *knowledge ) const;                                             
    virtual void RunNormalLinkedQueryMMed( const SolutionMap *hypothesis_links,
                                           const TheKnowledge *knowledge ) const;                                                                                

    virtual SYM::Over<SYM::BooleanExpression> SymbolicNormalLinkedQueryImpl() const;                                       
    virtual SYM::Over<SYM::BooleanExpression> SymbolicNormalLinkedQueryMMed() const;                                       
};


class PreRestrictedAgent : public DefaultMMAXAgent
{
public:    
    virtual void RunDecidedQueryMMed( DecidedQueryAgentInterface &query,
                                      XLink keyer_xlink ) const;                                      
    virtual void RunDecidedQueryPRed( DecidedQueryAgentInterface &query,
                                      XLink keyer_xlink ) const = 0;                                          
                                      
    virtual void RunNormalLinkedQueryMMed( const SolutionMap *hypothesis_links,
                                           const TheKnowledge *knowledge ) const;
    virtual void RunNormalLinkedQueryPRed( const SolutionMap *hypothesis_links,
                                           const TheKnowledge *knowledge ) const;                                      

    virtual SYM::Over<SYM::BooleanExpression> SymbolicNormalLinkedQueryMMed() const;                                       
    virtual SYM::Over<SYM::BooleanExpression> SymbolicNormalLinkedQueryPRed() const;                                       
};


class TeleportAgent : public PreRestrictedAgent
{
public:    
    virtual void RunDecidedQueryPRed( DecidedQueryAgentInterface &query,
                                      XLink keyer_xlink ) const;                  
    virtual map<PatternLink, XLink> RunTeleportQuery( XLink keyer_xlink ) const { ASSERTFAIL(); }
    
    virtual set<XLink> ExpandNormalDomain( const unordered_set<XLink> &keyer_xlinks );

    virtual void Reset();    

private:
    mutable CacheByLocation cache;    
};


class SearchLeafAgent : public PreRestrictedAgent
{
    virtual shared_ptr<PatternQuery> GetPatternQuery() const;
    virtual void RunDecidedQueryPRed( DecidedQueryAgentInterface &query,
                                      XLink keyer_xlink ) const;                  
};

};

#endif