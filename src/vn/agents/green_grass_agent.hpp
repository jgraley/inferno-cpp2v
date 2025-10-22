#ifndef GREEN_GRASS_AGENT_HPP
#define GREEN_GRASS_AGENT_HPP

#include "common/common.hpp"
#include "autolocating_agent.hpp"
#include "../sym/expression.hpp"
#include "../sym/predicate_operators.hpp"

namespace VN
{ 

/// Agent that only matches if the subtree at `through` is part of the original
/// input tree, i.e. during the second and later hits within the current step,
/// it does not match any part of the working graph that was created by an earlier
/// replace operation in the same step. This departs from the reductive style, 
/// so should be used with care.
class GreenGrassAgent : public virtual AutolocatingAgent 
{               
    virtual shared_ptr<PatternQuery> GetPatternQuery() const;
    virtual SYM::Lazy<SYM::BooleanExpression> SymbolicAutolocatingQuery(PatternLink keyer_plink) const; 

	Syntax::Production GetAgentProduction() const override;
	string GetRender( const RenderKit &kit, Syntax::Production surround_prod ) const final;
	
	NodeBlock GetGraphBlockInfo() const final;
    virtual const TreePtrInterface *GetThrough() const = 0;

    class IsGreenGrassOperator : public SYM::PredicateOperator
    {
    public:    
        typedef BooleanExpression NominalType;
        explicit IsGreenGrassOperator( const GreenGrassAgent *agent,
                                       shared_ptr<SYM::SymbolExpression> a); 
        shared_ptr<SYM::PredicateOperator> Clone() const override;
                                       
        list<shared_ptr<SYM::SymbolExpression> *> GetSymbolOperandPointers() override;
        virtual unique_ptr<SYM::BooleanResult> Evaluate( const EvalKit &kit,
                                                         list<unique_ptr<SYM::SymbolicResult>> &&op_results ) const override;

        Orderable::Diff OrderCompare3WayCovariant( const Orderable &right, 
                                               OrderProperty order_property ) const override;

        virtual string RenderNF() const override;
        virtual Precedence GetPrecedenceNF() const override;
        
    protected:
        shared_ptr<SYM::SymbolExpression> a;
        const GreenGrassAgent *agent;
    };
};


template<class PRE_RESTRICTION>
class GreenGrass : public GreenGrassAgent, 
                   public Special<PRE_RESTRICTION>
{
public:
    SPECIAL_NODE_FUNCTIONS
    
    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }
    
    TreePtr<PRE_RESTRICTION> through;
    virtual const TreePtrInterface *GetThrough() const
    {
        return &through;
    }
    string GetCouplingNameHint() const final
    {
		return "green"; 
	} 
};

};

#endif
