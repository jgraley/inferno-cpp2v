#ifndef STAR_AGENT_HPP
#define STAR_AGENT_HPP

#include "agent_common.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "../sym/expression.hpp"
#include "../sym/predicate_operators.hpp"
#include "special_agent.hpp"

namespace VN
{ 

/// Agent used in a container pattern under a standard agent, that matches 
/// zero or more elements
/// Star can match more than one node of any type in a container
/// In a Sequence, only a contiguous subsequence of 0 or more elements will match
/// In a Collection, a sub-collection of 0 or more elements may be matched anywhere in the collection
/// Only one Star is allowed in a Collection. Star must be templated on a type that is allowed
/// in the collection.
/// `restriction` may point to a sub-pattern that must match all nodes 
/// within the subcollection matched by the star node. Some limitations exist regarding
/// the limitations of the use of restrictions in mutliple star nodes. TODO: clarify.
/// The restriction is an abnormal context because it can match zero or more 
/// different subtrees.
class StarAgent : public virtual AgentCommon 
{
public:
    class PreRestrictionMismatch : public Mismatch {};
    class NotASubcontainerMismatch : public Mismatch {};

    virtual shared_ptr<PatternQuery> GetPatternQuery() const;                
    SYM::Lazy<SYM::BooleanExpression> SymbolicNormalLinkedQuery(PatternLink keyer_plink) const override;                                       
    virtual void RunRegenerationQueryImpl( DecidedQueryAgentInterface &query,
                                           const SolutionMap *hypothesis_links,
                                           PatternLink keyer_plink,
                                           const XTreeDatabase *x_tree_db ) const;                                                                                          

    ReplacePatchPtr GenReplaceLayoutImpl( const ReplaceKit &kit, 
                                       PatternLink me_plink, 
                                       XLink key_xlink,
                                                  const SCREngine *acting_engine ) final;

	Syntax::Production GetAgentProduction() const override;
	string GetRender( const RenderKit &kit, Syntax::Production surround_prod ) const final;
    NodeBlock GetGraphBlockInfo() const final;
    
private:
    virtual const TreePtrInterface *GetRestriction() const = 0;

    class IsSubcontainerInCategoryOperator : public SYM::IsInCategoryOperator
    {
        using IsInCategoryOperator::IsInCategoryOperator; 
        virtual unique_ptr<SYM::BooleanResult> Evaluate( const EvalKit &kit,
                                                         list<unique_ptr<SYM::SymbolicResult>> &&op_results ) const override;
        virtual string RenderNF() const override;
    };

public:
};


/// Agent used in a container pattern that matches zero or more elements
template<class PRE_RESTRICTION>
class Star : public StarAgent, 
             public Special<PRE_RESTRICTION>
{
public:
    SPECIAL_NODE_FUNCTIONS

    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }
    
    TreePtr<PRE_RESTRICTION> restriction; 
    virtual const TreePtrInterface *GetRestriction() const
    {
        return &restriction;
    }
    
    string GetCouplingNameHint() const final
    {
		return PRE_RESTRICTION::GetCouplingNameHint() + "s"; // Pluralise
	}    
};

};

#endif
