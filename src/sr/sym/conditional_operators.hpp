#ifndef CONDITIONAL_OPERATORS_HPP
#define CONDITIONAL_OPERATORS_HPP

#include "expression.hpp"
#include "overloads.hpp"

#include "../equivalence.hpp"
#include "../the_knowledge.hpp"

#include "common/common.hpp"
#include "common/read_args.hpp"

namespace SYM
{ 

// ------------------------- ConditionalOperator --------------------------

class ConditionalOperator : public SymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    explicit ConditionalOperator( shared_ptr<BooleanExpression> control,
                                  shared_ptr<SymbolExpression> option_true,
                                  shared_ptr<SymbolExpression> option_false );
    virtual list<shared_ptr<Expression>> GetOperands() const override;
    virtual shared_ptr<SymbolResultInterface> Evaluate( const EvalKit &kit ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const shared_ptr<BooleanExpression> control;
    const shared_ptr<SymbolExpression> option_true;
    const shared_ptr<SymbolExpression> option_false;
};

// ------------------------- MultiConditionalOperator --------------------------

class MultiConditionalOperator : public SymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    // Controls evaluated and then taken as a little-endian binary nuber which
    // indexes into options - this option is then evaluated and returned. No smart
    // handling of undefined controls yet.
    explicit MultiConditionalOperator( vector<shared_ptr<BooleanExpression>> controls,
                                       vector<shared_ptr<SymbolExpression>> options );
    virtual list<shared_ptr<Expression>> GetOperands() const override;
    virtual shared_ptr<SymbolResultInterface> Evaluate( const EvalKit &kit ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const vector<shared_ptr<BooleanExpression>> controls;
    const vector<shared_ptr<SymbolExpression>> options;
};

};

#endif // include guard
