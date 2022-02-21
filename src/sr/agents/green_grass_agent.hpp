#ifndef GREEN_GRASS_AGENT_HPP
#define GREEN_GRASS_AGENT_HPP

#include "common/common.hpp"
#include "colocated_agent.hpp"
#include "../sym/expression.hpp"
#include "../sym/predicate_operators.hpp"

namespace SR
{ 

/// Agent that only matches if the subtree at `through` is part of the original
/// input tree, i.e. durng the second and later hits of the master or any slave,
/// it does not match any part of the working graph that was created by an earlier
/// pass. This departs from the reductive style, so should be used with care.
class GreenGrassAgent : public virtual ColocatedAgent 
{               
    virtual shared_ptr<PatternQuery> GetPatternQuery() const;
    virtual bool ImplHasSNLQ() const;
    virtual SYM::Over<SYM::BooleanExpression> SymbolicColocatedQuery() const; 
    virtual Block GetGraphBlockInfo() const;
    virtual const TreePtrInterface *GetThrough() const = 0;

    class IsGreenGrassOperator : public SYM::SymbolToBooleanExpression
    {
    public:    
        typedef BooleanExpression NominalType;
        explicit IsGreenGrassOperator( const set< TreePtr<Node> > *dirty_grass,
                                       shared_ptr<SYM::SymbolExpression> a); 
        virtual list<shared_ptr<SYM::SymbolExpression>> GetSymbolOperands() const override;
        virtual shared_ptr<SYM::BooleanResultInterface> Evaluate( const EvalKit &kit,
                                                         const list<shared_ptr<SYM::SymbolResultInterface>> &op_results ) const override;

        virtual Orderable::Result OrderCompareLocal( const Orderable *candidate, 
                                                     OrderProperty order_property ) const override;

        virtual string Render() const override;
        virtual Precedence GetPrecedence() const override;
        
    protected:
        const shared_ptr<SYM::SymbolExpression> a;
        const set< TreePtr<Node> > * const dirty_grass;
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
};

};

#endif
