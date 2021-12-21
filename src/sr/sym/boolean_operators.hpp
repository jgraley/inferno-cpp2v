#ifndef BOOLEAN_OPERATORS_HPP
#define BOOLEAN_OPERATORS_HPP

#include "expression.hpp"
#include "lazy.hpp"

#include "common/common.hpp"
#include "common/read_args.hpp"

namespace SYM
{ 

// ------------------------- AndOperator --------------------------

class AndOperator : public BooleanToBooleanExpression
{
public:    
    typedef BooleanExpression NominalType;
    AndOperator( list< shared_ptr<BooleanExpression> > sa_ );
    virtual list<shared_ptr<BooleanExpression>> GetBooleanOperands() const override;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                const list<unique_ptr<BooleanResult>> &op_results ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    list< shared_ptr<BooleanExpression> > sa;
};

Lazy<BooleanExpression> operator&( Lazy<BooleanExpression> a, Lazy<BooleanExpression> b );

// ------------------------- OrOperator --------------------------

class OrOperator : public BooleanToBooleanExpression
{
public:    
    typedef BooleanExpression NominalType;
    OrOperator( list< shared_ptr<BooleanExpression> > sa_ );
    virtual list<shared_ptr<BooleanExpression>> GetBooleanOperands() const override;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                const list<unique_ptr<BooleanResult>> &op_results ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    list< shared_ptr<BooleanExpression> > sa;
};

Lazy<BooleanExpression> operator|( Lazy<BooleanExpression> a, Lazy<BooleanExpression> b );

};

#endif // include guard
