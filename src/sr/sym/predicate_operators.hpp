#ifndef PREDICATE_OPERATORS_HPP
#define PREDICATE_OPERATORS_HPP

#include "expression.hpp"
#include "overloads.hpp"

#include "../the_knowledge.hpp"
#include "../sc_relation.hpp"

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

// ------------------------- IsEqualOperator --------------------------

// This is an equation when assumed to evaluate to true. See #527
class IsEqualOperator : public PredicateOperator
{
public:    
    typedef BooleanExpression NominalType;
    explicit IsEqualOperator( shared_ptr<SymbolExpression> a, 
                            shared_ptr<SymbolExpression> b );
    IsEqualOperator *Clone() const override;

    list<shared_ptr<SymbolExpression> *> GetSymbolOperandPointers() override;
    unique_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                        list<unique_ptr<SymbolResultInterface>> &&op_results ) const override;
    bool IsCommutative() const override;

    shared_ptr<SymbolExpression> TrySolveFor( const SolveKit &kit, shared_ptr<SymbolVariable> target ) const override;
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
    VariablesRequiringNuggets GetVariablesRequiringNuggets() const override;
    list<shared_ptr<SymbolExpression> *> GetSymbolOperandPointers() override;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                list<unique_ptr<SymbolResultInterface>> &&op_results ) const override final;
    shared_ptr<SymbolExpression> TrySolveFor( const SolveKit &kit, shared_ptr<SymbolVariable> target ) const override;
    virtual list<shared_ptr<SymbolExpression>> GetRanges() const = 0;
    virtual bool EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                      SR::TheKnowledge::IndexType index_b ) const = 0;
    virtual Precedence GetPrecedenceNF() const override;
    
protected:
    shared_ptr<SymbolExpression> a;
    shared_ptr<SymbolExpression> b;
};

// ------------------------- IsGreaterOperator --------------------------

class IsGreaterOperator : public IndexComparisonOperator
{
    using IndexComparisonOperator::IndexComparisonOperator;
    IsGreaterOperator *Clone() const override;

    virtual bool EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                      SR::TheKnowledge::IndexType index_b ) const override;
    list<shared_ptr<SymbolExpression>> GetRanges() const override;
                                      
    Relationship GetRelationshipWith( shared_ptr<PredicateOperator> other ) const override;
    Transitivity GetTransitivityWith( shared_ptr<PredicateOperator> other ) const override;
    
    virtual string RenderNF() const override;
};

Over<BooleanExpression> operator>( Over<SymbolExpression> a, Over<SymbolExpression> b );

// ------------------------- IsLessOperator --------------------------

class IsLessOperator : public IndexComparisonOperator
{
    using IndexComparisonOperator::IndexComparisonOperator;
    IsLessOperator *Clone() const override;

    virtual bool EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                      SR::TheKnowledge::IndexType index_b ) const override;
    list<shared_ptr<SymbolExpression>> GetRanges() const override;

    Relationship GetRelationshipWith( shared_ptr<PredicateOperator> other ) const override;
    Transitivity GetTransitivityWith( shared_ptr<PredicateOperator> other ) const override;
    
    virtual string RenderNF() const override;
};

Over<BooleanExpression> operator<( Over<SymbolExpression> a, Over<SymbolExpression> b );

// ------------------------- IsGreaterOrEqualOperator --------------------------

class IsGreaterOrEqualOperator : public IndexComparisonOperator
{
    using IndexComparisonOperator::IndexComparisonOperator;
    IsGreaterOrEqualOperator *Clone() const override;

    virtual bool EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                      SR::TheKnowledge::IndexType index_b ) const override;
    list<shared_ptr<SymbolExpression>> GetRanges() const override;

    Relationship GetRelationshipWith( shared_ptr<PredicateOperator> other ) const override;
    Transitivity GetTransitivityWith( shared_ptr<PredicateOperator> other ) const override;
    
    virtual string RenderNF() const override;
};

Over<BooleanExpression> operator>=( Over<SymbolExpression> a, Over<SymbolExpression> b );

// ------------------------- IsLessOrEqualOperator --------------------------

class IsLessOrEqualOperator : public IndexComparisonOperator
{
    using IndexComparisonOperator::IndexComparisonOperator;
    IsLessOrEqualOperator *Clone() const override;

    virtual bool EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                      SR::TheKnowledge::IndexType index_b ) const override;
    list<shared_ptr<SymbolExpression>> GetRanges() const override;

    Relationship GetRelationshipWith( shared_ptr<PredicateOperator> other ) const override;
    Transitivity GetTransitivityWith( shared_ptr<PredicateOperator> other ) const override;
    
    virtual string RenderNF() const override;
};

Over<BooleanExpression> operator<=( Over<SymbolExpression> a, Over<SymbolExpression> b );

// ------------------------- IsAllDiffOperator --------------------------

class IsAllDiffOperator : public PredicateOperator
{
public:    
    typedef BooleanExpression NominalType;
    explicit IsAllDiffOperator( list< shared_ptr<SymbolExpression> > sa );
    IsAllDiffOperator *Clone() const override;

    list<shared_ptr<SymbolExpression> *> GetSymbolOperandPointers() override;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                list<unique_ptr<SymbolResultInterface>> &&op_results ) const override;

    shared_ptr<SymbolExpression> TrySolveFor( const SolveKit &kit, shared_ptr<SymbolVariable> target ) const override;
    bool IsCommutative() const override;
    Relationship GetRelationshipWith( shared_ptr<PredicateOperator> other ) const override;
    
    virtual string RenderNF() const override;
    virtual Precedence GetPrecedenceNF() const override;
    
private:
    list< shared_ptr<SymbolExpression> > sa;
};

// ------------------------- IsKindOfOperator --------------------------

// Note: this predicate is kept specific (rather than generalising to 
// eg IsInCategoryRange) because it needs to be seen by knowledge planning
// and only then can it be solved into the more generic AllInCategoryRangeOperator. 
class IsKindOfOperator : public PredicateOperator
{
public:    
    typedef BooleanExpression NominalType;
    explicit IsKindOfOperator( TreePtr<Node> archetype_node,
                             shared_ptr<SymbolExpression> a); 
    IsKindOfOperator *Clone() const override;

    list<shared_ptr<SymbolExpression> *> GetSymbolOperandPointers() override;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                list<unique_ptr<SymbolResultInterface>> &&op_results ) const override;
    shared_ptr<SymbolExpression> TrySolveFor( const SolveKit &kit, shared_ptr<SymbolVariable> target ) const override;
    Relationship GetRelationshipWith( shared_ptr<PredicateOperator> other ) const override;

    virtual Orderable::Result OrderCompareLocal( const Orderable *candidate, 
                                                 OrderProperty order_property ) const override;

    // TODO non-overlapping categories, might be easier with lace-ability, see #510
    //Relationship GetRelationshipWith( shared_ptr<PredicateOperator> other ) const override;

    virtual string RenderNF() const override;
    virtual Precedence GetPrecedenceNF() const override;
    TreePtr<Node> GetArchetypeNode() const;

protected:
    shared_ptr<SymbolExpression> a;
    const TreePtr<Node> archetype_node;
};

// ------------------------- IsCollectionSizedOperator --------------------------

class IsCollectionSizedOperator : public PredicateOperator
{
public:    
    typedef BooleanExpression NominalType;
    explicit IsCollectionSizedOperator( TreePtr<Node> archetype_node,
                                          int item, 
                                          shared_ptr<SymbolExpression> a,
                                          int size );
    IsCollectionSizedOperator *Clone() const override;

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

// ------------------------- IsSimpleCompareEquivalentOperator --------------------------

// Note: this predicate is kept specific (rather than generalising to 
// eg IsInSimpleCompareRangeOperator or via IsMemeberOperator) so that its
// solving properties can be exposed more easily eg transitivity. 
class IsSimpleCompareEquivalentOperator : public PredicateOperator
{
public:    
    typedef BooleanExpression NominalType;
    explicit IsSimpleCompareEquivalentOperator( shared_ptr<SymbolExpression> a, 
                                 shared_ptr<SymbolExpression> b );
    IsSimpleCompareEquivalentOperator *Clone() const override;

    list<shared_ptr<SymbolExpression> *> GetSymbolOperandPointers() override;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                list<unique_ptr<SymbolResultInterface>> &&op_results ) const override;
    
    shared_ptr<SymbolExpression> TrySolveFor( const SolveKit &kit, shared_ptr<SymbolVariable> target ) const override;
    bool IsCommutative() const override;
    Transitivity GetTransitivityWith( shared_ptr<PredicateOperator> other ) const override;
    
    virtual string RenderNF() const override;
    virtual Precedence GetPrecedenceNF() const override;
    
private:
    shared_ptr<SymbolExpression> a;
    shared_ptr<SymbolExpression> b;
    SR::SimpleCompareRelation equivalence_relation;
};

};

#endif // include guard
