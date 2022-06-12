#ifndef SET_OPERATORS_HPP
#define SET_OPERATORS_HPP

#include "expression.hpp"
#include "overloads.hpp"
#include "result.hpp"
#include "../the_knowledge.hpp"

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
    virtual unique_ptr<SymbolResultInterface> Evaluate( const EvalKit &kit,
                                                        list<unique_ptr<SymbolResultInterface>> &&op_results ) const override final;
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
    virtual unique_ptr<SymbolResultInterface> Evaluate( const EvalKit &kit,
                                                        list<unique_ptr<SymbolResultInterface>> &&op_results ) const override;

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
    virtual unique_ptr<SymbolResultInterface> Evaluate( const EvalKit &kit,
                                                        list<unique_ptr<SymbolResultInterface>> &&op_results ) const override;

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
    VariablesRequiringNuggets GetVariablesRequiringNuggets() const override;
    list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    unique_ptr<SymbolResultInterface> Evaluate( const EvalKit &kit,
                                                list<unique_ptr<SymbolResultInterface>> &&op_results ) const final;
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
    VariablesRequiringNuggets GetVariablesRequiringNuggets() const override;
    list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    unique_ptr<SymbolResultInterface> Evaluate( const EvalKit &kit,
                                                list<unique_ptr<SymbolResultInterface>> &&op_results ) const final;
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
    VariablesRequiringNuggets GetVariablesRequiringNuggets() const override;
    list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    unique_ptr<SymbolResultInterface> Evaluate( const EvalKit &kit,
                                                list<unique_ptr<SymbolResultInterface>> &&op_results ) const final;
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
    VariablesRequiringNuggets GetVariablesRequiringNuggets() const override;
    list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    unique_ptr<SymbolResultInterface> Evaluate( const EvalKit &kit,
                                                list<unique_ptr<SymbolResultInterface>> &&op_results ) const final;
    string Render() const override;
    Precedence GetPrecedence() const override;
    
private:
    const shared_ptr<SymbolExpression> a;
};

// ------------------------- AllCouplingEquivalentOperator --------------------------

// Equivalence is under Simple Compare of Child X of a pair of XLinks. Actual 
// root arrow-head identity is ignored. There is no ordering, only equivalence 
// relation. This is sufficient for couplings. Rule #528
class AllCouplingEquivalentOperator : public SymbolToSymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    AllCouplingEquivalentOperator( shared_ptr<SymbolExpression> a );
    list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    unique_ptr<SymbolResultInterface> Evaluate( const EvalKit &kit,
                                                list<unique_ptr<SymbolResultInterface>> &&op_results ) const final;
    string Render() const override;
    Precedence GetPrecedence() const override;
    
private:
    const shared_ptr<SymbolExpression> a;
};

// ------------------------- AllInCategoryRange --------------------------

class AllInCategoryRange : public SYM::SymbolToSymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    AllInCategoryRange( CategoryRangeResult::XLinkBoundsList &&bounds_list, bool lower_incl, bool upper_incl );
    list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    unique_ptr<SYM::SymbolResultInterface> Evaluate( const EvalKit &kit,
                                                     list<unique_ptr<SYM::SymbolResultInterface>> &&op_results ) const final;
    string Render() const override;
    Precedence GetPrecedence() const override;
    
private:
    CategoryRangeResult::XLinkBoundsList bounds_list;
    const bool lower_incl, upper_incl;
};

// ------------------------- AllInSimpleCompareRange --------------------------

class AllInSimpleCompareRange : public SYM::SymbolToSymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    AllInSimpleCompareRange( pair<TreePtr<Node>, TreePtr<Node>> &&bounds, bool lower_incl, bool upper_incl );
    list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    unique_ptr<SYM::SymbolResultInterface> Evaluate( const EvalKit &kit,
                                                     list<unique_ptr<SYM::SymbolResultInterface>> &&op_results ) const final;
    string Render() const override;
    Precedence GetPrecedence() const override;
    
private:
    pair<TreePtr<Node>, TreePtr<Node>> bounds;
    const bool lower_incl, upper_incl;
};

};

#endif // include guard
