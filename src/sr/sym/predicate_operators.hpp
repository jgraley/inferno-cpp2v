#ifndef PREDICATE_OPERATORS_HPP
#define PREDICATE_OPERATORS_HPP

#include "expression.hpp"
#include "overloads.hpp"

#include "../the_knowledge.hpp"

#include "common/common.hpp"
#include "common/read_args.hpp"

namespace SYM
{ 

// ----------------------------- enums -------------------------------

enum class Relationship
{
    NONE,
    CONTRADICTS, // Pi ∧ Pj is false  (symmetrical)
    IMPLIES        // Pi => Pj          (asymmetrical)
};


enum class Transitivity
{
    NONE,
    FORWARD,      // S1 ✕ S2 ∧ S2 ⚬ S3 = S1 ✕ S3
    REVERSE,      // S1 ✕ S2 ∧ S3 ⚬ S2 = S1 ✕ S3
    BIDIRECTIONAL // Either of the above is true
};

// ------------------------- PredicateOperator --------------------------

class PredicateOperator : public SymbolToBooleanExpression
{
public:    
    typedef SymbolToBooleanExpression Parent;
            
    virtual PredicateOperator *Clone() const = 0;
    
    // Stored as weak pointer so that force is undone when weak pointer expires. Sort of RAII-at-a-distance.
    void SetForceExpression( weak_ptr<BooleanExpression> force_expression );
    void SetForceRender( weak_ptr<string> force_render ); // Note: precedence goes to LITERAL

    list<shared_ptr<SymbolExpression>> GetSymbolOperands() const final;
    virtual list<shared_ptr<SymbolExpression> *> GetSymbolOperandPointers() = 0;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit ) const override;
    
    shared_ptr<PredicateOperator> TrySubstitute( shared_ptr<SymbolExpression> over,
                                                 shared_ptr<SymbolExpression> with ) const;
    virtual Relationship GetRelationshipWith( shared_ptr<PredicateOperator> other ) const;
    virtual Transitivity GetTransitivityWith( shared_ptr<PredicateOperator> other ) const;
    virtual bool IsCanSubstituteFrom() const;

    string Render() const override final;
    Precedence GetPrecedence() const override final;
        
    // Methods to use if not forced
    virtual string RenderNF() const = 0;    
    virtual Precedence GetPrecedenceNF() const = 0;
    
private:    
    weak_ptr<BooleanExpression> force_expression;
    weak_ptr<string> force_render;
};

// ------------------------- EqualOperator --------------------------

// This is an equation when assumed to evaluate to true. See #527
class EqualOperator : public PredicateOperator
{
public:    
    typedef BooleanExpression NominalType;
    explicit EqualOperator( shared_ptr<SymbolExpression> a, 
                            shared_ptr<SymbolExpression> b );
    EqualOperator *Clone() const override;

    list<shared_ptr<SymbolExpression> *> GetSymbolOperandPointers() override;
    unique_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                        list<unique_ptr<SymbolResultInterface>> &&op_results ) const override;
    bool IsCommutative() const override;

    shared_ptr<Expression> TrySolveForToEqualNT( shared_ptr<Expression> target, 
                                                 shared_ptr<BooleanExpression> to_equal ) const override;
    Relationship GetRelationshipWith( shared_ptr<PredicateOperator> other ) const override;
    bool IsCanSubstituteFrom() const override;

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
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                list<unique_ptr<SymbolResultInterface>> &&op_results ) const override final;
    shared_ptr<Expression> TrySolveForToEqualNT( shared_ptr<Expression> target, 
                                                 shared_ptr<BooleanExpression> to_equal ) const override;
    virtual pair<shared_ptr<SymbolExpression>, shared_ptr<SymbolExpression>> GetRanges() const = 0;
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
    pair<shared_ptr<SymbolExpression>, shared_ptr<SymbolExpression>> GetRanges() const override;
                                      
    Relationship GetRelationshipWith( shared_ptr<PredicateOperator> other ) const override;
    Transitivity GetTransitivityWith( shared_ptr<PredicateOperator> other ) const override;
    
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
    pair<shared_ptr<SymbolExpression>, shared_ptr<SymbolExpression>> GetRanges() const override;

    Relationship GetRelationshipWith( shared_ptr<PredicateOperator> other ) const override;
    Transitivity GetTransitivityWith( shared_ptr<PredicateOperator> other ) const override;
    
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
    pair<shared_ptr<SymbolExpression>, shared_ptr<SymbolExpression>> GetRanges() const override;

    Relationship GetRelationshipWith( shared_ptr<PredicateOperator> other ) const override;
    Transitivity GetTransitivityWith( shared_ptr<PredicateOperator> other ) const override;
    
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
    pair<shared_ptr<SymbolExpression>, shared_ptr<SymbolExpression>> GetRanges() const override;

    Relationship GetRelationshipWith( shared_ptr<PredicateOperator> other ) const override;
    Transitivity GetTransitivityWith( shared_ptr<PredicateOperator> other ) const override;
    
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
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                list<unique_ptr<SymbolResultInterface>> &&op_results ) const override;

    shared_ptr<Expression> TrySolveForToEqualNT( shared_ptr<Expression> target, 
                                                 shared_ptr<BooleanExpression> to_equal ) const override;
    bool IsCommutative() const override;
    Relationship GetRelationshipWith( shared_ptr<PredicateOperator> other ) const override;
    
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
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                list<unique_ptr<SymbolResultInterface>> &&op_results ) const override;

    virtual Orderable::Result OrderCompareLocal( const Orderable *candidate, 
                                                 OrderProperty order_property ) const override;

    // TODO non-overlapping categories, might be easier with lace-ability, see #510
    //Relationship GetRelationshipWith( shared_ptr<PredicateOperator> other ) const override;

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
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                list<unique_ptr<SymbolResultInterface>> &&op_results ) const override final;

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

// ------------------------- IsCouplingEquivalentOperator --------------------------

class IsCouplingEquivalentOperator : public PredicateOperator
{
public:    
    typedef BooleanExpression NominalType;
    explicit IsCouplingEquivalentOperator( shared_ptr<SymbolExpression> a, 
                                 shared_ptr<SymbolExpression> b );
    IsCouplingEquivalentOperator *Clone() const override;

    list<shared_ptr<SymbolExpression> *> GetSymbolOperandPointers() override;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                list<unique_ptr<SymbolResultInterface>> &&op_results ) const override;
    
    shared_ptr<Expression> TrySolveForToEqualNT( shared_ptr<Expression> target, 
                                                 shared_ptr<BooleanExpression> to_equal ) const override;
    bool IsCommutative() const override;
    Transitivity GetTransitivityWith( shared_ptr<PredicateOperator> other ) const override;
    
    virtual string RenderNF() const override;
    virtual Precedence GetPrecedenceNF() const override;
    
private:
    shared_ptr<SymbolExpression> a;
    shared_ptr<SymbolExpression> b;
    SR::TheKnowledge::CouplingRelation equivalence_relation;
};

};

#endif // include guard
