#ifndef PREDICATE_OPERATORS_HPP
#define PREDICATE_OPERATORS_HPP

#include "expression.hpp"
#include "overloads.hpp"

#include "../equivalence.hpp"
#include "../the_knowledge.hpp"

#include "common/common.hpp"
#include "common/read_args.hpp"

namespace SYM
{ 

// ------------------------- PredicateOperator --------------------------

class PredicateOperator : public SymbolToBooleanExpression
{
public:    
    typedef SymbolToBooleanExpression Parent;
    
    virtual PredicateOperator *Clone() const = 0;
    
    // Stored as weak pointer so that force is undone when weak pointer expires. Sort of RAII-at-a-distance.
    void SetForceResult( weak_ptr<BooleanResultInterface> force_result );
    void SetForceRender( weak_ptr<string> force_render ); // Note: precedence goes to LITERAL

    list<shared_ptr<SymbolExpression>> GetSymbolOperands() const final;
    virtual list<shared_ptr<SymbolExpression> *> GetSymbolOperandPointers() = 0;
    virtual shared_ptr<BooleanResultInterface> Evaluate( const EvalKit &kit ) const override;

    virtual shared_ptr<PredicateOperator> TryDerive( shared_ptr<PredicateOperator> other ) const;
    virtual shared_ptr<PredicateOperator> TrySubstitute( shared_ptr<SymbolExpression> over,
                                                         shared_ptr<SymbolExpression> with ) const;
    string Render() const override final;
    Precedence GetPrecedence() const override final;
        
    // Methods to use if not forced
    virtual string RenderNF() const = 0;    
    virtual Precedence GetPrecedenceNF() const = 0;
    
private:    
    weak_ptr<BooleanResultInterface> force_result;
    weak_ptr<string> force_render;
};

// ------------------------- EqualOperator --------------------------

// EqualOperator is in fact a more general "AllSame" operator, which 
// seems to make natural sense.
class EqualOperator : public PredicateOperator
{
public:    
    typedef BooleanExpression NominalType;
    explicit EqualOperator( shared_ptr<SymbolExpression> a, 
                            shared_ptr<SymbolExpression> b );
    EqualOperator *Clone() const override;

    list<shared_ptr<SymbolExpression> *> GetSymbolOperandPointers() override;
    shared_ptr<BooleanResultInterface> Evaluate( const EvalKit &kit,
                                                 const list<shared_ptr<SymbolResultInterface>> &op_results ) const override;
    bool IsCommutative() const override;

    shared_ptr<Expression> TrySolveForToEqualNT( shared_ptr<Expression> target, 
                                                         shared_ptr<BooleanExpression> to_equal ) const override;
    shared_ptr<PredicateOperator> TryDerive( shared_ptr<PredicateOperator> other ) const override;
    shared_ptr<PredicateOperator> TrySubstitute( shared_ptr<SymbolExpression> over,
                                                 shared_ptr<SymbolExpression> with ) const override;
                                                         
    string RenderNF() const override;
    Precedence GetPrecedenceNF() const override;
    
private:
    shared_ptr<SymbolExpression> a;
    shared_ptr<SymbolExpression> b;
};


Over<BooleanExpression> operator==( Over<SymbolExpression> a, Over<SymbolExpression> b );

// ------------------------- (not equal operator) --------------------------

Over<BooleanExpression> operator!=( Over<SymbolExpression> a, Over<SymbolExpression> b );

// ------------------------- IndexComparisonOperator --------------------------

// Use depth-first index from the knowledge to effect inequalities
class IndexComparisonOperator : public PredicateOperator
{
public:    
    typedef BooleanExpression NominalType;
    explicit IndexComparisonOperator( shared_ptr<SymbolExpression> a_, 
                                      shared_ptr<SymbolExpression> b_ );
    list<shared_ptr<SymbolExpression> *> GetSymbolOperandPointers() override;
    virtual shared_ptr<BooleanResultInterface> Evaluate( const EvalKit &kit,
                                                const list<shared_ptr<SymbolResultInterface>> &op_results ) const override final;
    virtual bool EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                      SR::TheKnowledge::IndexType index_b ) const = 0;
    virtual Precedence GetPrecedenceNF() const override;
    
protected:
    shared_ptr<SymbolExpression> a;
    shared_ptr<SymbolExpression> b;
};

// ------------------------- GreaterOperator --------------------------

class GreaterOperator : public IndexComparisonOperator
{
    using IndexComparisonOperator::IndexComparisonOperator;
    GreaterOperator *Clone() const override;

    virtual bool EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                      SR::TheKnowledge::IndexType index_b ) const override;
    virtual string RenderNF() const override;
};

Over<BooleanExpression> operator>( Over<SymbolExpression> a, Over<SymbolExpression> b );

// ------------------------- LessOperator --------------------------

class LessOperator : public IndexComparisonOperator
{
    using IndexComparisonOperator::IndexComparisonOperator;
    LessOperator *Clone() const override;

    virtual bool EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                      SR::TheKnowledge::IndexType index_b ) const override;
    virtual string RenderNF() const override;
};

Over<BooleanExpression> operator<( Over<SymbolExpression> a, Over<SymbolExpression> b );

// ------------------------- GreaterOrEqualOperator --------------------------

class GreaterOrEqualOperator : public IndexComparisonOperator
{
    using IndexComparisonOperator::IndexComparisonOperator;
    GreaterOrEqualOperator *Clone() const override;

    virtual bool EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                      SR::TheKnowledge::IndexType index_b ) const override;
    virtual string RenderNF() const override;
};

Over<BooleanExpression> operator>=( Over<SymbolExpression> a, Over<SymbolExpression> b );

// ------------------------- LessOrEqualOperator --------------------------

class LessOrEqualOperator : public IndexComparisonOperator
{
    using IndexComparisonOperator::IndexComparisonOperator;
    LessOrEqualOperator *Clone() const override;

    virtual bool EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                      SR::TheKnowledge::IndexType index_b ) const override;
    virtual string RenderNF() const override;
};

Over<BooleanExpression> operator<=( Over<SymbolExpression> a, Over<SymbolExpression> b );

// ------------------------- AllDiffOperator --------------------------

class AllDiffOperator : public PredicateOperator
{
public:    
    typedef BooleanExpression NominalType;
    explicit AllDiffOperator( list< shared_ptr<SymbolExpression> > sa );
    AllDiffOperator *Clone() const override;

    list<shared_ptr<SymbolExpression> *> GetSymbolOperandPointers() override;
    virtual shared_ptr<BooleanResultInterface> Evaluate( const EvalKit &kit,
                                                const list<shared_ptr<SymbolResultInterface>> &op_results ) const override;
    bool IsCommutative() const override;
    virtual string RenderNF() const override;
    virtual Precedence GetPrecedenceNF() const override;
    
private:
    list< shared_ptr<SymbolExpression> > sa;
};

// ------------------------- KindOfOperator --------------------------

class KindOfOperator : public PredicateOperator
{
public:    
    typedef BooleanExpression NominalType;
    explicit KindOfOperator( TreePtr<Node> archetype_node,
                             shared_ptr<SymbolExpression> a); 
    KindOfOperator *Clone() const override;

    list<shared_ptr<SymbolExpression> *> GetSymbolOperandPointers() override;
    virtual shared_ptr<BooleanResultInterface> Evaluate( const EvalKit &kit,
                                                const list<shared_ptr<SymbolResultInterface>> &op_results ) const override;

    virtual Orderable::Result OrderCompareLocal( const Orderable *candidate, 
                                                 OrderProperty order_property ) const override;

    virtual string RenderNF() const override;
    virtual Precedence GetPrecedenceNF() const override;
    
protected:
    shared_ptr<SymbolExpression> a;
    const TreePtr<Node> archetype_node;
};

// ------------------------- ChildCollectionSizeOperator --------------------------

class ChildCollectionSizeOperator : public PredicateOperator
{
public:    
    typedef BooleanExpression NominalType;
    explicit ChildCollectionSizeOperator( TreePtr<Node> archetype_node,
                                          int item, 
                                          shared_ptr<SymbolExpression> a,
                                          int size );
    ChildCollectionSizeOperator *Clone() const override;

    list<shared_ptr<SymbolExpression> *> GetSymbolOperandPointers() override;
    virtual shared_ptr<BooleanResultInterface> Evaluate( const EvalKit &kit,
                                                const list<shared_ptr<SymbolResultInterface>> &op_results ) const override final;

    virtual Orderable::Result OrderCompareLocal( const Orderable *candidate, 
                                                 OrderProperty order_property ) const override;                                                

    virtual string RenderNF() const override;
    virtual Precedence GetPrecedenceNF() const override;
    
private:
    const TreePtr<Node> archetype_node;
    const int item_index;
    shared_ptr<SymbolExpression> a;
    const int size;
};

// ------------------------- EquivalentOperator --------------------------

class EquivalentOperator : public PredicateOperator
{
public:    
    typedef BooleanExpression NominalType;
    explicit EquivalentOperator( shared_ptr<SymbolExpression> a, 
                                 shared_ptr<SymbolExpression> b );
    EquivalentOperator *Clone() const override;

    list<shared_ptr<SymbolExpression> *> GetSymbolOperandPointers() override;
    virtual shared_ptr<BooleanResultInterface> Evaluate( const EvalKit &kit,
                                                const list<shared_ptr<SymbolResultInterface>> &op_results ) const override;
    bool IsCommutative() const override;
    shared_ptr<PredicateOperator> TrySubstitute( shared_ptr<SymbolExpression> over,
                                                 shared_ptr<SymbolExpression> with ) const override;
    virtual string RenderNF() const override;
    virtual Precedence GetPrecedenceNF() const override;
    
private:
    shared_ptr<SymbolExpression> a;
    shared_ptr<SymbolExpression> b;
    SR::EquivalenceRelation equivalence_relation;
};

};

#endif // include guard
