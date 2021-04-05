#ifndef DISJUNCTION_AGENT_HPP
#define DISJUNCTION_AGENT_HPP

#include "../search_replace.hpp"
#include "../boolean_evaluator.hpp"

namespace SR
{

/// Boolean node that matches if any of the sub-patterns at the pointers in
/// `patterns` do match i.e. an "or" operation. `patterns` point to abnormal 
/// contexts since in an overall match, some sub-patterns may not match.
class DisjunctionAgent : public virtual PreRestrictedAgent 
{
public:
    class NoOptionsMatchedMismatch : public Mismatch {};
    class TakenOptionMismatch : public Mismatch {};
    class MMAXRequiredOnUntakenOptionMismatch : public Mismatch {};

    virtual shared_ptr<PatternQuery> GetPatternQuery() const;
    virtual void RunDecidedQueryPRed( DecidedQueryAgentInterface &query,
                                      XLink x ) const;                                    
                                                               
    virtual bool ImplHasNLQ() const;
    virtual bool NLQRequiresBase() const; 
    virtual void RunNormalLinkedQueryPRed( const SolutionMap *required_links,
                                           const TheKnowledge *knowledge ) const;                
                                                                         
    virtual Block GetGraphBlockInfo( const LinkNamingFunction &lnf,
                                     const NonTrivialPreRestrictionFunction &ntprf ) const;
    
private:
    virtual CollectionInterface &GetPatterns() const = 0;
    virtual void AgentConfigure( Phase phase, const SCREngine *master_scr_engine );
    shared_ptr< Collection<Node> > options;
};


template<class PRE_RESTRICTION>
class Disjunction : public Special<PRE_RESTRICTION>,
                    public DisjunctionAgent
{
public:
    SPECIAL_NODE_FUNCTIONS
    
    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }
    
    // Patterns are an abnormal context
    mutable Collection<PRE_RESTRICTION> patterns; // TODO provide const iterators and remove mutable
private:
    virtual CollectionInterface &GetPatterns() const
    {
        return patterns;
    }
};

    
};

#endif
