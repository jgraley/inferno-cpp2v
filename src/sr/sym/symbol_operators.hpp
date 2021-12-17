#ifndef SYMBOL_OPERATORS_HPP
#define SYMBOL_OPERATORS_HPP

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

// ------------------------- SingularChildOperator --------------------------

class SingularChildOperator : public SymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    SingularChildOperator( const SR::Agent *ref_agent,
                           int item, 
                           shared_ptr<SymbolExpression> a );
    virtual set<shared_ptr<Expression>> GetOperands() const override;
    virtual unique_ptr<SymbolResult> Evaluate( const EvalKit &kit ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const SR::Agent *ref_agent;
    const int item;
    shared_ptr<SymbolExpression> a;
};

};

#endif // include guard
