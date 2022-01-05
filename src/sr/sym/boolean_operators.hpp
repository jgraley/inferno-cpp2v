#ifndef BOOLEAN_OPERATORS_HPP
#define BOOLEAN_OPERATORS_HPP

#include "expression.hpp"
#include "overloads.hpp"

#include "common/common.hpp"
#include "common/read_args.hpp"

namespace SYM
{ 

// ------------------------- AndOperator --------------------------

class AndOperator : public BooleanToBooleanExpression
{
public:    
    typedef BooleanExpression NominalType;
    explicit AndOperator( list< shared_ptr<BooleanExpression> > sa_ );
    virtual list<shared_ptr<BooleanExpression>> GetBooleanOperands() const override;
    virtual shared_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                const list<shared_ptr<BooleanResult>> &op_results ) const override;
    virtual shared_ptr<SymbolExpression> TrySolveFor( shared_ptr<SymbolVariable> target ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const list< shared_ptr<BooleanExpression> > sa;
};

Over<BooleanExpression> operator&( Over<BooleanExpression> a, Over<BooleanExpression> b );

// ------------------------- OrOperator --------------------------

class OrOperator : public BooleanToBooleanExpression
{
public:    
    typedef BooleanExpression NominalType;
    explicit OrOperator( list< shared_ptr<BooleanExpression> > sa_ );
    virtual list<shared_ptr<BooleanExpression>> GetBooleanOperands() const override;
    virtual shared_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                const list<shared_ptr<BooleanResult>> &op_results ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const list< shared_ptr<BooleanExpression> > sa;
};

Over<BooleanExpression> operator|( Over<BooleanExpression> a, Over<BooleanExpression> b );

// ------------------------- BoolEqualOperator --------------------------

class BoolEqualOperator : public BooleanToBooleanExpression
{
public:    
    typedef BooleanExpression NominalType;
    explicit BoolEqualOperator( list< shared_ptr<BooleanExpression> > sa_ );
    virtual list<shared_ptr<BooleanExpression>> GetBooleanOperands() const override;
    virtual shared_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                const list<shared_ptr<BooleanResult>> &op_results ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const list< shared_ptr<BooleanExpression> > sa;
};

Over<BooleanExpression> operator==( Over<BooleanExpression> a, Over<BooleanExpression> b );

// ------------------------- ImplicationOperator --------------------------

class ImplicationOperator : public BooleanToBooleanExpression
{
public:    
    typedef BooleanExpression NominalType;
    explicit ImplicationOperator( shared_ptr<BooleanExpression> a_,
                                  shared_ptr<BooleanExpression> b_ );
    virtual list<shared_ptr<BooleanExpression>> GetBooleanOperands() const override;
    virtual shared_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                const list<shared_ptr<BooleanResult>> &op_results ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const shared_ptr<BooleanExpression> a;
    const shared_ptr<BooleanExpression> b;
};

};

#endif // include guard
