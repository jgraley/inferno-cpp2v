#ifndef BOOLEAN_OPERATORS_HPP
#define BOOLEAN_OPERATORS_HPP

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
    virtual BooleanResult Evaluate( const EvalKit &kit ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    set< shared_ptr<SymbolExpression> > sa;
};


Lazy<BooleanExpression> operator==( Lazy<SymbolExpression> a, Lazy<SymbolExpression> b );

// ------------------------- PreRestrictionOperator --------------------------

class PreRestrictionOperator : public BooleanExpression
{
public:    
    typedef BooleanExpression EvalType;
    PreRestrictionOperator( shared_ptr<SymbolExpression> a,
                            const SR::Agent *pre_restrictor ); 
    virtual set<shared_ptr<Expression>> GetOperands() const override;
    virtual BooleanResult Evaluate( const EvalKit &kit ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    shared_ptr<SymbolExpression> a;
    const SR::Agent *pre_restrictor; // TODO Would like to use an archetype to reduce coupling
};

};

#endif // include guard
