#ifndef COMPARISON_OPERATORS_HPP
#define COMPARISON_OPERATORS_HPP

#include "expression.hpp"
#include "overloads.hpp"

#include "../equivalence.hpp"

#include "common/common.hpp"
#include "common/read_args.hpp"

namespace SR
{
    class Agent;
};

namespace SYM
{ 

// ------------------------- EqualOperator --------------------------

// EqualOperator is in fact a more general "AllSame" operator, which 
// seems to make natural sense.
class EqualOperator : public SymbolToBooleanExpression
{
public:    
    typedef BooleanExpression NominalType;
    explicit EqualOperator( list< shared_ptr<SymbolExpression> > sa );
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    virtual shared_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                const list<shared_ptr<SymbolResult>> &op_results ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const list< shared_ptr<SymbolExpression> > sa;
};


Over<BooleanExpression> operator==( Over<SymbolExpression> a, Over<SymbolExpression> b );

// ------------------------- NotEqualOperator --------------------------

// NotEqualOperator is NOT a more general "AllDiff" operator; it only 
// accepts 2 operands and if you need AllDiff you should try 
// AllDiffOperator. It isn't 100% clear that AllDiff is the natural
// interpretation of a multi-operand !=
class NotEqualOperator : public SymbolToBooleanExpression
{
public:    
    typedef BooleanExpression NominalType;
    explicit NotEqualOperator( shared_ptr<SymbolExpression> a_, 
                               shared_ptr<SymbolExpression> b_ );
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    virtual shared_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                const list<shared_ptr<SymbolResult>> &op_results ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const shared_ptr<SymbolExpression> a;
    const shared_ptr<SymbolExpression> b;
};


Over<BooleanExpression> operator!=( Over<SymbolExpression> a, Over<SymbolExpression> b );

// ------------------------- IndexComparisonOperator --------------------------

class IndexComparisonOperator : public SymbolToBooleanExpression
{
public:    
    typedef BooleanExpression NominalType;
    explicit IndexComparisonOperator( shared_ptr<SymbolExpression> a_, 
                                      shared_ptr<SymbolExpression> b_ );
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    virtual shared_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                const list<shared_ptr<SymbolResult>> &op_results ) const override final;
    virtual bool EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                      SR::TheKnowledge::IndexType index_b ) const = 0;
    virtual Precedence GetPrecedence() const override;
    
protected:
    const shared_ptr<SymbolExpression> a;
    const shared_ptr<SymbolExpression> b;
};

// ------------------------- GreaterOperator --------------------------

class GreaterOperator : public IndexComparisonOperator
{
    using IndexComparisonOperator::IndexComparisonOperator;
    virtual bool EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                      SR::TheKnowledge::IndexType index_b ) const override;
    virtual string Render() const override;
};

Over<BooleanExpression> operator>( Over<SymbolExpression> a, Over<SymbolExpression> b );

// ------------------------- LessOperator --------------------------

class LessOperator : public IndexComparisonOperator
{
    using IndexComparisonOperator::IndexComparisonOperator;
    virtual bool EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                      SR::TheKnowledge::IndexType index_b ) const override;
    virtual string Render() const override;
};

Over<BooleanExpression> operator<( Over<SymbolExpression> a, Over<SymbolExpression> b );

// ------------------------- GreaterOrEqualOperator --------------------------

class GreaterOrEqualOperator : public IndexComparisonOperator
{
    using IndexComparisonOperator::IndexComparisonOperator;
    virtual bool EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                      SR::TheKnowledge::IndexType index_b ) const override;
    virtual string Render() const override;
};

Over<BooleanExpression> operator>=( Over<SymbolExpression> a, Over<SymbolExpression> b );

// ------------------------- LessOrEqualOperator --------------------------

class LessOrEqualOperator : public IndexComparisonOperator
{
    using IndexComparisonOperator::IndexComparisonOperator;
    virtual bool EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                      SR::TheKnowledge::IndexType index_b ) const override;
    virtual string Render() const override;
};

Over<BooleanExpression> operator<=( Over<SymbolExpression> a, Over<SymbolExpression> b );

// ------------------------- AllDiffOperator --------------------------

class AllDiffOperator : public SymbolToBooleanExpression
{
public:    
    typedef BooleanExpression NominalType;
    explicit AllDiffOperator( list< shared_ptr<SymbolExpression> > sa );
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    virtual shared_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                const list<shared_ptr<SymbolResult>> &op_results ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const list< shared_ptr<SymbolExpression> > sa;
};

// ------------------------- KindOfOperator --------------------------

class KindOfOperator : public SymbolToBooleanExpression
{
public:    
    typedef BooleanExpression NominalType;
    explicit KindOfOperator( const SR::Agent *ref_agent,
                              shared_ptr<SymbolExpression> a); 
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    virtual shared_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                const list<shared_ptr<SymbolResult>> &op_results ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const shared_ptr<SymbolExpression> a;
    const SR::Agent * const ref_agent; // TODO Would like to use an archetype to reduce coupling
};

// ------------------------- ChildCollectionSizeOperator --------------------------

class ChildCollectionSizeOperator : public SymbolToBooleanExpression
{
public:    
    typedef BooleanExpression NominalType;
    explicit ChildCollectionSizeOperator( const SR::Agent *ref_agent,
                                          int item, 
                                          shared_ptr<SymbolExpression> a,
                                          int size );
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    virtual shared_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                const list<shared_ptr<SymbolResult>> &op_results ) const override final;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const SR::Agent *ref_agent;
    const int item_index;
    const shared_ptr<SymbolExpression> a;
    const int size;
};

// ------------------------- EquivalentOperator --------------------------

class EquivalentOperator : public SymbolToBooleanExpression
{
public:    
    typedef BooleanExpression NominalType;
    explicit EquivalentOperator( list< shared_ptr<SymbolExpression> > sa );
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    virtual shared_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                const list<shared_ptr<SymbolResult>> &op_results ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const list< shared_ptr<SymbolExpression> > sa;
    SR::EquivalenceRelation equivalence_relation;
};

};

#endif // include guard
