#ifndef NOT_MATCH_AGENT_HPP
#define NOT_MATCH_AGENT_HPP

#include "../search_replace.hpp"
#include "helpers/transformation.hpp"
#include "overlay_agent.hpp"
#include "slave_agent.hpp"
#include "../boolean_evaluator.hpp"

namespace SR
{

/// Boolean node that matches if the sub-pattern at `pattern` does 
/// not match i.e. a "not" operation. `pattern` points to an abnormal 
/// context since in an overall match, the sub-pattern does not match.
class NotMatchAgent : public virtual AgentCommon 
{
public:
    virtual shared_ptr<PatternQuery> GetPatternQuery() const;
    virtual void RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                      TreePtr<Node> x ) const;                  
	virtual void GetGraphAppearance( bool *bold, string *text, string *shape ) const;
    
private:
    virtual const TreePtrInterface *GetPattern() const = 0;
    class BooleanEvaluatorNot : public BooleanEvaluator
    {
	public:
   	    virtual bool operator()( list<bool> &inputs ) const;
	};
};


template<class PRE_RESTRICTION>
class NotMatch : public Special<PRE_RESTRICTION>,
                 public NotMatchAgent
{
public:
    SPECIAL_NODE_FUNCTIONS
    // Pattern is an abnormal context. Fascinatingly, we can supply any node here because there
    // is no type-correctness limitation with *excluding* a kind of node. Use the virtual
    // GetPattern() anyway, for consistency.
    TreePtr<Node> pattern;
private:
    virtual const TreePtrInterface *GetPattern() const
    {
        return &pattern;
    }
};
    
};

#endif
