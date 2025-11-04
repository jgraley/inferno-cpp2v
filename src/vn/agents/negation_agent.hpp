#ifndef NEGATION_AGENT_HPP
#define NEGATION_AGENT_HPP

#include "agent_intermediates.hpp"
#include "../search_replace.hpp"
#include "../boolean_evaluator.hpp"
#include "special_agent.hpp"
#include "autolocating_agent.hpp"

namespace VN
{

/// Boolean node that matches if the sub-pattern at `pattern` does 
/// not match i.e. a "not" operation. `pattern` points to an abnormal 
/// context since in an overall match, the sub-pattern does not match.
class NegationAgent : public virtual AutolocatingAgent 
{
public:
    SPECIAL_NODE_FUNCTIONS
    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }
    
    class LocationMismatch : public Agent::Mismatch {};

    virtual shared_ptr<PatternQuery> GetPatternQuery() const;              

    virtual void RunRegenerationQueryImpl( DecidedQueryAgentInterface &query,
                                           const SolutionMap *hypothesis_links,
                                           PatternLink keyer_plink,
                                           const XTreeDatabase *x_tree_db ) const;                                              

	Syntax::Production GetAgentProduction() const override;
	string GetRender( const RenderKit &kit, Syntax::Production surround_prod ) const final;
    NodeBlock GetGraphBlockInfo() const final;

    TreePtr<Node> negand;    
    const TreePtrInterface *GetNegand() const
    {
        return &negand;
    }

    string GetCouplingNameHint() const final
    {
		return "not"; 
	}
};
  
};

#endif
