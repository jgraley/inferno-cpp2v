#ifndef COMPARISON_OPERATORS_HPP
#define COMPARISON_OPERATORS_HPP

#include "expression.hpp"
#include "lazy.hpp"

#include "common/common.hpp"
#include "common/read_args.hpp"

namespace SR
{
    class Agent;
};

namespace SYM
{ 

// ------------------------- EqualsOperator --------------------------

class EqualsOperator : public BooleanExpression
{
public:    
    typedef BooleanExpression EvalType;
    EqualsOperator( set< shared_ptr<SymbolExpression> > sa );
    virtual set<shared_ptr<Expression>> GetOperands() const override;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    set< shared_ptr<SymbolExpression> > sa;
};


Lazy<BooleanExpression> operator==( Lazy<SymbolExpression> a, Lazy<SymbolExpression> b );

// ------------------------- NotEqualsOperator --------------------------

class NotEqualsOperator : public BooleanExpression
{
public:    
    typedef BooleanExpression EvalType;
    NotEqualsOperator( set< shared_ptr<SymbolExpression> > sa );
    virtual set<shared_ptr<Expression>> GetOperands() const override;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    set< shared_ptr<SymbolExpression> > sa;
};


Lazy<BooleanExpression> operator!=( Lazy<SymbolExpression> a, Lazy<SymbolExpression> b );

// ------------------------- KindOfOperator --------------------------

class KindOfOperator : public BooleanExpression
{
public:    
    typedef BooleanExpression EvalType;
    KindOfOperator( const SR::Agent *ref_agent,
                    shared_ptr<SymbolExpression> a); 
    virtual set<shared_ptr<Expression>> GetOperands() const override;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    shared_ptr<SymbolExpression> a;
    const SR::Agent *ref_agent; // TODO Would like to use an archetype to reduce coupling
};

};

#endif // include guard
