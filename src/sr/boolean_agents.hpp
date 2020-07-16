#ifndef BOOLEAN_AGENTS_HPP
#define BOOLEAN_AGENTS_HPP

#include "search_replace.hpp"
#include "helpers/transformation.hpp"
#include "overlay_agent.hpp"
#include "slave_agent.hpp"
#include "boolean_evaluator.hpp"

namespace SR
{

/// Boolean agents permit simple logic operations to be applied to one
/// or more sub-patterns, which then become the matching status of the 
/// overall pattern.
class BooleanAgent : public AgentCommon 
{
};
    
//---------------------------------- NotMatch ------------------------------------    
/// Boolean node that matches if the sub-pattern at `pattern` does 
/// not match i.e. a "not" operation. `pattern` points to an abnormal 
/// context since in an overall match, the sub-pattern does not match.
class NotMatchAgent : public BooleanAgent 
{
public:
    virtual PatternQueryResult PatternQuery() const;
    virtual DecidedQueryResult DecidedQuery( const TreePtrInterface *px,
                                             const AgentQuery::Choices &choices ) const;
	virtual void GetGraphAppearance( bool *bold, string *text, string *shape ) const;
    
private:
    virtual TreePtr<Node> GetPattern() const = 0;
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
    virtual TreePtr<Node> GetPattern() const
    {
        return (TreePtr<Node>)pattern;
    }
};


//---------------------------------- MatchAll ------------------------------------    
/// Boolean node that matches if all of the sub-patterns at the pointers in
/// `patterns` do match i.e. an "and" operation. `patterns` point to  
/// normal contexts, since the global and-rule is preserved.
class MatchAllAgent : public BooleanAgent 
{
public:
    virtual PatternQueryResult PatternQuery() const;
    virtual DecidedQueryResult DecidedQuery( const TreePtrInterface *px,
                                             const AgentQuery::Choices &choices ) const;
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


//---------------------------------- MatchAny ------------------------------------    
/// Boolean node that matches if any of the sub-patterns at the pointers in
/// `patterns` do match i.e. an "or" operation. `patterns` point to abnormal 
/// contexts since in an overall match, some sub-patterns may not match.
class MatchAnyAgent : public BooleanAgent 
{
public:
    virtual PatternQueryResult PatternQuery() const;
    virtual DecidedQueryResult DecidedQuery( const TreePtrInterface *px,
                                             const AgentQuery::Choices &choices ) const;
	virtual void GetGraphAppearance( bool *bold, string *text, string *shape ) const;
private:
    virtual CollectionInterface &GetPatterns() const = 0;
    class BooleanEvaluatorOr : public BooleanEvaluator
    {
	public:
   	    virtual bool operator()( list<bool> &inputs ) const;
	};
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
