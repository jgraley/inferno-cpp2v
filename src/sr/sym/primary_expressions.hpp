#ifndef PRIMARY_EXPRESSIONS_HPP
#define PRIMARY_EXPRESSIONS_HPP

#include "expression.hpp"
#include "lazy.hpp"

#include "common/common.hpp"
#include "common/read_args.hpp"

namespace SYM
{ 

// ------------------------- Constant --------------------------

class Constant : public SymbolExpression
{
public:    
    typedef SymbolExpression EvalType;
    Constant( SR::XLink xlink );
    virtual SymbolResult Evaluate( const EvalKit &kit ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    SR::XLink xlink;
};

// ------------------------- Variable --------------------------

class Variable : public SymbolExpression
{
public:    
    typedef SymbolExpression EvalType;
    Variable( SR::PatternLink plink );
    virtual set<SR::PatternLink> GetRequiredPatternLinks() const override;
    virtual SymbolResult Evaluate( const EvalKit &kit ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    SR::PatternLink plink;
};

};

#endif // include guard
