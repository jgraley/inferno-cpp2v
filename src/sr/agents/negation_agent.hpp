#ifndef NEGATION_AGENT_HPP
#define NEGATION_AGENT_HPP

#include "../search_replace.hpp"
#include "../boolean_evaluator.hpp"

namespace SR
{

/// Boolean node that matches if the sub-pattern at `pattern` does 
/// not match i.e. a "not" operation. `pattern` points to an abnormal 
/// context since in an overall match, the sub-pattern does not match.
class NegationAgent : public virtual PreRestrictedAgent 
{
public:
    class LocationMismatch : public Agent::Mismatch {};

    virtual shared_ptr<PatternQuery> GetPatternQuery() const;
    virtual void RunDecidedQueryPRed( DecidedQueryAgentInterface &query,
                                      XLink x ) const;                  
    virtual void RunRegenerationQueryImpl( DecidedQueryAgentInterface &query,
                                           XLink base_xlink,
                                           const SolutionMap *required_links,
                                           const TheKnowledge *knowledge ) const;                                              
    virtual Block GetGraphBlockInfo( const LinkNamingFunction &lnf ) const;
    
private:
    virtual const TreePtrInterface *GetPattern() const = 0;
};


template<class PRE_RESTRICTION>
class Negation : public Special<PRE_RESTRICTION>,
                 public NegationAgent
{
public:
    SPECIAL_NODE_FUNCTIONS

    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }
    
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
