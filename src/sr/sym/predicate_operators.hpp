#ifndef PREDICATE_OPERATORS_HPP
#define PREDICATE_OPERATORS_HPP

#include "expression.hpp"
#include "lazy_eval.hpp"

#include "../db/x_tree_database.hpp"
#include "helpers/simple_compare.hpp"
#include "../db/orderings.hpp"

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
            
    virtual shared_ptr<PredicateOperator> Clone() const = 0;
    
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
    shared_ptr<PredicateOperator> Clone() const override;

    list<shared_ptr<SymbolExpression> *> GetSymbolOperandPointers() override;
    unique_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                        list<unique_ptr<SymbolicResult>> &&op_results ) const override;
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


Lazy<BooleanExpression> operator==( Lazy<SymbolExpression> a, Lazy<SymbolExpression> b );

// ------------------------- (not equal operator) --------------------------

Lazy<BooleanExpression> operator!=( Lazy<SymbolExpression> a, Lazy<SymbolExpression> b );

// ------------------------- DepthFirstComparisonOperator --------------------------

// Use depth-first index from the x_tree_db to effect inequalities
class DepthFirstComparisonOperator : public PredicateOperator
{
public:    
    typedef BooleanExpression NominalType;
    explicit DepthFirstComparisonOperator( shared_ptr<SymbolExpression> a_, 
                                      shared_ptr<SymbolExpression> b_ );
    VariablesRequiringRows GetVariablesRequiringRows() const override;
    list<shared_ptr<SymbolExpression> *> GetSymbolOperandPointers() override;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                list<unique_ptr<SymbolicResult>> &&op_results ) const override final;
    shared_ptr<SymbolExpression> TrySolveFor( const SolveKit &kit, shared_ptr<SymbolVariable> target ) const override;
    virtual list<shared_ptr<SymbolExpression>> GetRanges() const = 0;
    virtual bool EvalBoolFromDiff( Orderable::Diff diff ) const = 0;
    virtual Precedence GetPrecedenceNF() const override;
    
protected:
    shared_ptr<SymbolExpression> a;
    shared_ptr<SymbolExpression> b;
};

// ------------------------- IsGreaterOperator --------------------------

class IsGreaterOperator : public DepthFirstComparisonOperator
{
    using DepthFirstComparisonOperator::DepthFirstComparisonOperator;
    shared_ptr<PredicateOperator> Clone() const override;

    bool EvalBoolFromDiff( Orderable::Diff diff ) const override;
    list<shared_ptr<SymbolExpression>> GetRanges() const override;
                                      
    Relationship GetRelationshipWith( shared_ptr<PredicateOperator> other ) const override;
    Transitivity GetTransitivityWith( shared_ptr<PredicateOperator> other ) const override;
    
    virtual string RenderNF() const override;
};

Lazy<BooleanExpression> operator>( Lazy<SymbolExpression> a, Lazy<SymbolExpression> b );

// ------------------------- IsLessOperator --------------------------

class IsLessOperator : public DepthFirstComparisonOperator
{
    using DepthFirstComparisonOperator::DepthFirstComparisonOperator;
    shared_ptr<PredicateOperator> Clone() const override;

    bool EvalBoolFromDiff( Orderable::Diff diff ) const override;
    list<shared_ptr<SymbolExpression>> GetRanges() const override;

    Relationship GetRelationshipWith( shared_ptr<PredicateOperator> other ) const override;
    Transitivity GetTransitivityWith( shared_ptr<PredicateOperator> other ) const override;
    
    virtual string RenderNF() const override;
};

Lazy<BooleanExpression> operator<( Lazy<SymbolExpression> a, Lazy<SymbolExpression> b );

// ------------------------- IsGreaterOrEqualOperator --------------------------

class IsGreaterOrEqualOperator : public DepthFirstComparisonOperator
{
    using DepthFirstComparisonOperator::DepthFirstComparisonOperator;
    shared_ptr<PredicateOperator> Clone() const override;

    bool EvalBoolFromDiff( Orderable::Diff diff ) const override;
    list<shared_ptr<SymbolExpression>> GetRanges() const override;

    Relationship GetRelationshipWith( shared_ptr<PredicateOperator> other ) const override;
    Transitivity GetTransitivityWith( shared_ptr<PredicateOperator> other ) const override;
    
    virtual string RenderNF() const override;
};

Lazy<BooleanExpression> operator>=( Lazy<SymbolExpression> a, Lazy<SymbolExpression> b );

// ------------------------- IsLessOrEqualOperator --------------------------

class IsLessOrEqualOperator : public DepthFirstComparisonOperator
{
    using DepthFirstComparisonOperator::DepthFirstComparisonOperator;
    shared_ptr<PredicateOperator> Clone() const override;

    bool EvalBoolFromDiff( Orderable::Diff diff ) const override;
    list<shared_ptr<SymbolExpression>> GetRanges() const override;

    Relationship GetRelationshipWith( shared_ptr<PredicateOperator> other ) const override;
    Transitivity GetTransitivityWith( shared_ptr<PredicateOperator> other ) const override;
    
    virtual string RenderNF() const override;
};

Lazy<BooleanExpression> operator<=( Lazy<SymbolExpression> a, Lazy<SymbolExpression> b );

// ------------------------- IsAllDiffOperator --------------------------

class IsAllDiffOperator : public PredicateOperator
{
public:    
    typedef BooleanExpression NominalType;
    explicit IsAllDiffOperator( list< shared_ptr<SymbolExpression> > sa );
    shared_ptr<PredicateOperator> Clone() const override;

    list<shared_ptr<SymbolExpression> *> GetSymbolOperandPointers() override;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                list<unique_ptr<SymbolicResult>> &&op_results ) const override;

    shared_ptr<SymbolExpression> TrySolveFor( const SolveKit &kit, shared_ptr<SymbolVariable> target ) const override;
    bool IsCommutative() const override;
    Relationship GetRelationshipWith( shared_ptr<PredicateOperator> other ) const override;
    
    virtual string RenderNF() const override;
    virtual Precedence GetPrecedenceNF() const override;
    
private:
    list< shared_ptr<SymbolExpression> > sa;
};

// ------------------------- IsInCategoryOperator --------------------------

// Note: this predicate is kept specific (rather than generalising to 
// eg IsInCategoryRange) because it needs to be seen by x_tree_db planning
// and only then can it be solved into the more generic AllInCategoryRangeOperator. 
class IsInCategoryOperator : public PredicateOperator
{
public:    
    typedef BooleanExpression NominalType;
    explicit IsInCategoryOperator( TreePtr<Node> archetype_node,
                             shared_ptr<SymbolExpression> a); 
    shared_ptr<PredicateOperator> Clone() const override;

    list<shared_ptr<SymbolExpression> *> GetSymbolOperandPointers() override;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                list<unique_ptr<SymbolicResult>> &&op_results ) const override;
    shared_ptr<SymbolExpression> TrySolveFor( const SolveKit &kit, shared_ptr<SymbolVariable> target ) const override;
    Relationship GetRelationshipWith( shared_ptr<PredicateOperator> other ) const override;

    Orderable::Diff OrderCompare3WayCovariant( const Orderable &right, 
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

// ------------------------- IsChildCollectionSizedOperator --------------------------

class IsChildCollectionSizedOperator : public PredicateOperator
{
public:    
    typedef BooleanExpression NominalType;
    explicit IsChildCollectionSizedOperator( TreePtr<Node> archetype_node,
                                          vector< Itemiser::Element * >::size_type item_index_, 
                                          shared_ptr<SymbolExpression> a,
                                          int size );
    shared_ptr<PredicateOperator> Clone() const override;

    list<shared_ptr<SymbolExpression> *> GetSymbolOperandPointers() override;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                list<unique_ptr<SymbolicResult>> &&op_results ) const override final;

    Orderable::Diff OrderCompare3WayCovariant( const Orderable &right, 
                                           OrderProperty order_property ) const override;                                                

    virtual string RenderNF() const override;
    virtual Precedence GetPrecedenceNF() const override;
    
private:
    const TreePtr<Node> archetype_node;
    const vector< Itemiser::Element * >::size_type item_index;
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
    shared_ptr<PredicateOperator> Clone() const override;

    list<shared_ptr<SymbolExpression> *> GetSymbolOperandPointers() override;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                list<unique_ptr<SymbolicResult>> &&op_results ) const override;
    
    shared_ptr<SymbolExpression> TrySolveFor( const SolveKit &kit, shared_ptr<SymbolVariable> target ) const override;
    bool IsCommutative() const override;
    Transitivity GetTransitivityWith( shared_ptr<PredicateOperator> other ) const override;
    
    virtual string RenderNF() const override;
    virtual Precedence GetPrecedenceNF() const override;
    
private:
    shared_ptr<SymbolExpression> a;
    shared_ptr<SymbolExpression> b;
    SimpleCompare equivalence_relation;
};

// ------------------------- IsLocalMatchOperator --------------------------

class IsLocalMatchOperator : public PredicateOperator
{
public:    
    typedef BooleanExpression NominalType;
    explicit IsLocalMatchOperator( const Node *pattern_node, 
                                   shared_ptr<SymbolExpression> a );
    shared_ptr<PredicateOperator> Clone() const override;

    list<shared_ptr<SymbolExpression> *> GetSymbolOperandPointers() override;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                list<unique_ptr<SymbolicResult>> &&op_results ) const override final;

    Orderable::Diff OrderCompare3WayCovariant( const Orderable &right, 
                                           OrderProperty order_property ) const override;                                                

    virtual string RenderNF() const override;
    virtual Precedence GetPrecedenceNF() const override;
    
private:
    const Node *pattern_node;
    shared_ptr<SymbolExpression> a;
};


};

#endif // include guard
