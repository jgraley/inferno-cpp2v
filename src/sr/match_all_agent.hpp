#ifndef MATCH_ALL_AGENT_HPP
#define MATCH_ALL_AGENT_HPP

#include "search_replace.hpp"
#include "helpers/transformation.hpp"
#include "overlay_agent.hpp"
#include "slave_agent.hpp"
#include "boolean_evaluator.hpp"

namespace SR
{

/// Boolean node that matches if all of the sub-patterns at the pointers in
/// `patterns` do match i.e. an "and" operation. `patterns` point to  
/// normal contexts, since the global and-rule is preserved.
class MatchAllAgent : public virtual AgentCommon 
{
public:
    virtual shared_ptr<PatternQuery> GetPatternQuery() const;
    virtual void RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                               const TreePtrInterface *px ) const;                  
	virtual void GetGraphAppearance( bool *bold, string *text, string *shape ) const;
private:
    virtual CollectionInterface &GetPatterns() const = 0;
};


template<class PRE_RESTRICTION>
class MatchAll : public Special<PRE_RESTRICTION>,
                 public MatchAllAgent
{
public:
    SPECIAL_NODE_FUNCTIONS
    mutable Collection<PRE_RESTRICTION> patterns; // TODO provide const iterators and remove mutable
private:
    virtual CollectionInterface &GetPatterns() const
    {
        return patterns;
    }
};

};

#endif
