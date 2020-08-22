#ifndef MATCH_ANY_AGENT_HPP
#define MATCH_ANY_AGENT_HPP

#include "search_replace.hpp"
#include "helpers/transformation.hpp"
#include "overlay_agent.hpp"
#include "slave_agent.hpp"
#include "boolean_evaluator.hpp"

#define DECISIONISED_MATCH_ANY

namespace SR
{

/// Boolean node that matches if any of the sub-patterns at the pointers in
/// `patterns` do match i.e. an "or" operation. `patterns` point to abnormal 
/// contexts since in an overall match, some sub-patterns may not match.
class MatchAnyAgent : public virtual AgentCommon 
{
public:
    virtual shared_ptr<PatternQuery> GetPatternQuery() const;
    virtual void RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                               const TreePtrInterface *px ) const;                  
	virtual void GetGraphAppearance( bool *bold, string *text, string *shape ) const;
private:
    virtual CollectionInterface &GetPatterns() const = 0;
#ifdef DECISIONISED_MATCH_ANY
    virtual void AgentConfigure( const SCREngine *master_engine );
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
class MatchAny : public Special<PRE_RESTRICTION>,
                 public MatchAnyAgent
{
public:
    SPECIAL_NODE_FUNCTIONS
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
