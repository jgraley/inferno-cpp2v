#ifndef MATCH_ANY_AGENT_HPP
#define MATCH_ANY_AGENT_HPP

#include "../search_replace.hpp"
#include "helpers/transformation.hpp"
#include "overlay_agent.hpp"
#include "slave_agent.hpp"
#include "../boolean_evaluator.hpp"

#define DECISIONISED_MATCH_ANY

namespace SR
{

/// Boolean node that matches if any of the sub-patterns at the pointers in
/// `patterns` do match i.e. an "or" operation. `patterns` point to abnormal 
/// contexts since in an overall match, some sub-patterns may not match.
class DisjunctionAgent : public virtual AgentCommon 
{
public:
    class NoOptionsMatchedMismatch : public Mismatch {};
    class MMAXRequiredOnUntakenOptionMismatch : public Mismatch {};

    virtual shared_ptr<PatternQuery> GetPatternQuery() const;
    virtual void RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                      XLink x ) const;                                                             
    virtual bool ImplHasNLQ() const;
    virtual void RunNormalLinkedQueryImpl( XLink base_xlink,
                                           const SolutionMap *required_links,
                                           const TheKnowledge *knowledge ) const;                                              
    virtual Block GetGraphBlockInfo() const;
    
private:
    virtual CollectionInterface &GetPatterns() const = 0;
#ifdef DECISIONISED_MATCH_ANY
    virtual void AgentConfigure( Phase phase, const SCREngine *master_scr_engine );
    shared_ptr< Collection<Node> > options;
#else
    class BooleanEvaluatorOr : public BooleanEvaluator
    {
	public:
   	    virtual bool operator()( list<bool> &inputs ) const;
	};
#endif
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
