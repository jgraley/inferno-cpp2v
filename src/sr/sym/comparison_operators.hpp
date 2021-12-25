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

class EqualsOperator : public SymbolToBooleanExpression
{
public:    
    typedef BooleanExpression NominalType;
    explicit EqualsOperator( list< shared_ptr<SymbolExpression> > sa );
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    virtual shared_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                const list<shared_ptr<SymbolResult>> &op_results ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const list< shared_ptr<SymbolExpression> > sa;
};


Lazy<BooleanExpression> operator==( Lazy<SymbolExpression> a, Lazy<SymbolExpression> b );

// ------------------------- NotEqualsOperator --------------------------

class NotEqualsOperator : public SymbolToBooleanExpression
{
public:    
    typedef BooleanExpression NominalType;
    explicit NotEqualsOperator( list< shared_ptr<SymbolExpression> > sa );
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    virtual shared_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                const list<shared_ptr<SymbolResult>> &op_results ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const list< shared_ptr<SymbolExpression> > sa;
};


Lazy<BooleanExpression> operator!=( Lazy<SymbolExpression> a, Lazy<SymbolExpression> b );

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

};

#endif // include guard
