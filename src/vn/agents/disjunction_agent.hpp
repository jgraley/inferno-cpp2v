#ifndef DISJUNCTION_AGENT_HPP
#define DISJUNCTION_AGENT_HPP

#include "agent_common.hpp"
#include "autolocating_agent.hpp"
#include "../search_replace.hpp"
#include "../boolean_evaluator.hpp"
#include "standard_agent.hpp"
#include "../sym/predicate_operators.hpp"
#include "special_agent.hpp"

namespace VN
{

/// Boolean node that matches if any of the sub-patterns at the pointers in
/// `patterns` do match i.e. an "or" operation. `patterns` point to abnormal 
/// contexts since in an overall match, some sub-patterns may not match.
class DisjunctionAgent : public virtual AutolocatingAgent
{
public:
    SPECIAL_NODE_FUNCTIONS
    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }
    
    virtual shared_ptr<PatternQuery> GetPatternQuery() const;                                                                                               
    SYM::Lazy<SYM::BooleanExpression> SymbolicNormalLinkedQuery(PatternLink keyer_plink) const override;                                       

	Syntax::Production GetAgentProduction() const override;
	string GetRender( const RenderKit &kit, Syntax::Production surround_prod ) const final;
    NodeBlock GetGraphBlockInfo() const final;
    bool IsNonTrivialPreRestriction(const TreePtrInterface *pptr) const final;          
	bool IsFixedType() const final;
    
    // Interface for pattern trasformation
    TreePtr<Node> CloneToEmpty() const
    {
        auto agent_node = MakeTreeNode<DisjunctionAgent>();
        agent_node->pre_restriction_archetype_node = pre_restriction_archetype_node;
        agent_node->pre_restriction_archetype_ptr = pre_restriction_archetype_ptr;
        return agent_node;
    }
    
    // Patterns are an abnormal context
    mutable Collection<Node> disjuncts; // TODO provide const iterators and remove mutable
    CollectionInterface &GetDisjuncts() const
    {
        return disjuncts;
    }
    
    void SetDisjuncts( CollectionInterface &ci )
    {
        // Note: options should not have been set yet during ptrans so 
        // only need to update patterns
        disjuncts.clear();
        for( CollectionInterface::iterator pit = ci.begin(); 
             pit != ci.end(); 
             ++pit )    
            disjuncts.insert( *pit );      
    }
    
    string GetCouplingNameHint() const final
    {
		return "any"; 
	}

private:
    virtual void SCRConfigure( Phase phase );
    shared_ptr< Collection<Node> > options;
};


};

#endif
