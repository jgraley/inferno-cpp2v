#ifndef SET_OPERATORS_HPP
#define SET_OPERATORS_HPP

#include "expression.hpp"
#include "lazy_eval.hpp"
#include "result.hpp"
#include "../db/x_tree_database.hpp"

#include "common/common.hpp"
#include "common/read_args.hpp"

namespace SYM
{ 

// ------------------------- ComplementOperator --------------------------

class ComplementOperator : public SymbolToSymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    explicit ComplementOperator( shared_ptr<SymbolExpression> a );
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    virtual unique_ptr<SymbolicResult> Evaluate( const EvalKit &kit,
                                                        list<unique_ptr<SymbolicResult>> &&op_results ) const override final;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const shared_ptr<SymbolExpression> a;
};

// ------------------------- UnionOperator --------------------------

class UnionOperator : public SymbolToSymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    explicit UnionOperator( list< shared_ptr<SymbolExpression> > sa_ );
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    virtual unique_ptr<SymbolicResult> Evaluate( const EvalKit &kit,
                                                        list<unique_ptr<SymbolicResult>> &&op_results ) const override;

    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const list< shared_ptr<SymbolExpression> > sa;
};

// ------------------------- IntersectionOperator --------------------------

class IntersectionOperator : public SymbolToSymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    explicit IntersectionOperator( list< shared_ptr<SymbolExpression> > sa_ );
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    virtual unique_ptr<SymbolicResult> Evaluate( const EvalKit &kit,
                                                        list<unique_ptr<SymbolicResult>> &&op_results ) const override;

    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const list< shared_ptr<SymbolExpression> > sa;
};

// ------------------------- AllGreaterOperator --------------------------

class AllGreaterOperator : public SymbolToSymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    AllGreaterOperator( shared_ptr<SymbolExpression> a );
    VariablesRequiringRows GetVariablesRequiringRows() const override;
    list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    unique_ptr<SymbolicResult> Evaluate( const EvalKit &kit,
                                                list<unique_ptr<SymbolicResult>> &&op_results ) const final;
    string Render() const override;
    Precedence GetPrecedence() const override;
    
private:
    const shared_ptr<SymbolExpression> a;
};

// ------------------------- AllLessOperator --------------------------

class AllLessOperator : public SymbolToSymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    AllLessOperator( shared_ptr<SymbolExpression> a );
    VariablesRequiringRows GetVariablesRequiringRows() const override;
    list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    unique_ptr<SymbolicResult> Evaluate( const EvalKit &kit,
                                                list<unique_ptr<SymbolicResult>> &&op_results ) const final;
    string Render() const override;
    Precedence GetPrecedence() const override;
    
private:
    const shared_ptr<SymbolExpression> a;
};

// ------------------------- AllGreaterOrEqualOperator --------------------------

class AllGreaterOrEqualOperator : public SymbolToSymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    AllGreaterOrEqualOperator( shared_ptr<SymbolExpression> a );
    VariablesRequiringRows GetVariablesRequiringRows() const override;
    list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    unique_ptr<SymbolicResult> Evaluate( const EvalKit &kit,
                                                list<unique_ptr<SymbolicResult>> &&op_results ) const final;
    string Render() const override;
    Precedence GetPrecedence() const override;
    
private:
    const shared_ptr<SymbolExpression> a;
};

// ------------------------- AllLessOrEqualOperator --------------------------

class AllLessOrEqualOperator : public SymbolToSymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    AllLessOrEqualOperator( shared_ptr<SymbolExpression> a );
    VariablesRequiringRows GetVariablesRequiringRows() const override;
    list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    unique_ptr<SymbolicResult> Evaluate( const EvalKit &kit,
                                                list<unique_ptr<SymbolicResult>> &&op_results ) const final;
    string Render() const override;
    Precedence GetPrecedence() const override;
    
private:
    const shared_ptr<SymbolExpression> a;
};

// ------------------------- AllInSimpleCompareRangeOperator --------------------------

// Equivalence is under Simple Compare of Child X of a pair of XLinks. Actual 
// root arrow-head identity is ignored. There is no ordering, only equivalence 
// relation. This is sufficient for couplings. Rule #528
class AllInSimpleCompareRangeOperator : public SymbolToSymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    AllInSimpleCompareRangeOperator( shared_ptr<SymbolExpression> lower,
                                     Orderable::BoundingRole lower_role,
                                     bool lower_incl,
                                     shared_ptr<SymbolExpression> upper,
                                     Orderable::BoundingRole upper_role,
                                     bool upper_incl ); 
    list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    
    // Note we override the version without the operand solves - we'll do that here
    unique_ptr<SymbolicResult> Evaluate( const EvalKit &kit ) const final;
    string Render() const override;
    Precedence GetPrecedence() const override;
    
private:
    const shared_ptr<SymbolExpression> lower;
    const Orderable::BoundingRole lower_role;
    const bool lower_incl;
    const shared_ptr<SymbolExpression> upper;
    const Orderable::BoundingRole upper_role;
    const bool upper_incl;
};

// ------------------------- AllInCategoryRangeOperator --------------------------

class AllInCategoryRangeOperator : public SYM::SymbolToSymbolExpression
{
public:    
    typedef pair<shared_ptr<SymbolExpression>, shared_ptr<SymbolExpression>> ExprBounds;
    typedef list<ExprBounds> ExprBoundsList;

    typedef SymbolExpression NominalType;
    AllInCategoryRangeOperator( ExprBoundsList &&bounds_exprs_list_, bool lower_incl, bool upper_incl );
    list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    
    // Note we override the version without the operand solves - we'll do that here
    unique_ptr<SYM::SymbolicResult> Evaluate( const EvalKit &kit ) const final;
    string Render() const override;
    Precedence GetPrecedence() const override;
    
private:
    const ExprBoundsList bounds_exprs_list;
    const bool lower_incl, upper_incl;
};

};

#endif // include guard
