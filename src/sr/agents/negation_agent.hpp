#ifndef NEGATION_AGENT_HPP
#define NEGATION_AGENT_HPP

#include "agent_intermediates.hpp"
#include "../search_replace.hpp"
#include "../boolean_evaluator.hpp"
#include "special_agent.hpp"

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

    virtual SYM::Lazy<SYM::BooleanExpression> SymbolicNormalLinkedQueryPRed() const override;                                       

    virtual void RunRegenerationQueryImpl( DecidedQueryAgentInterface &query,
                                           const SolutionMap *hypothesis_links,
                                           const XTreeDatabase *x_tree_db ) const;                                              

    CommandPtr GenerateCommandImpl( const ReplaceKit &kit, 
                                    PatternLink me_plink, 
                                    XLink key_xlink ) final;

    virtual Block GetGraphBlockInfo() const;
    
private:
    virtual const TreePtrInterface *GetNegand() const = 0;
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
    // GetNegand() anyway, for consistency.
    TreePtr<Node> negand;
private:
    virtual const TreePtrInterface *GetNegand() const
    {
        return &negand;
    }
};
    
};

#endif
