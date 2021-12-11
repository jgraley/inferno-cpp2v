#ifndef PRIMARY_EXPRESSIONS_HPP
#define PRIMARY_EXPRESSIONS_HPP

#include "expression.hpp"
#include "lazy.hpp"

#include "common/common.hpp"
#include "common/read_args.hpp"

namespace SYM
{ 

// ------------------------- SymbolConstant --------------------------

class SymbolConstant : public SymbolExpression
{
public:    
    typedef SymbolExpression EvalType;
    SymbolConstant( SR::XLink xlink );
    virtual SymbolResult Evaluate( const EvalKit &kit ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    SR::XLink xlink;
};

// ------------------------- SymbolVariable --------------------------

class SymbolVariable : public SymbolExpression
{
public:    
    typedef SymbolExpression EvalType;
    SymbolVariable( SR::PatternLink plink );
    virtual set<SR::PatternLink> GetRequiredPatternLinks() const override;
    virtual SymbolResult Evaluate( const EvalKit &kit ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    SR::PatternLink plink;
};

// ------------------------- BooleanConstant --------------------------

class BooleanConstant : public BooleanExpression
{
public:    
    typedef BooleanExpression EvalType;
    BooleanConstant( bool value );
    virtual BooleanResult Evaluate( const EvalKit &kit ) const override;
    BooleanResult GetValue() const;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    BooleanResult::Matched matched;
};

};

#endif // include guard
