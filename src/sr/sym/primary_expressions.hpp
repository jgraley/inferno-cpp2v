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
    typedef SymbolExpression NominalType;
    SymbolConstant( SR::XLink xlink );
    virtual unique_ptr<SymbolResult> Evaluate( const EvalKit &kit ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    SR::XLink xlink;
};

// ------------------------- SymbolVariable --------------------------

class SymbolVariable : public SymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    SymbolVariable( const SR::PatternLink &plink );
    virtual set<SR::PatternLink> GetRequiredVariables() const override;
    virtual unique_ptr<SymbolResult> Evaluate( const EvalKit &kit ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    SR::PatternLink plink;
};

// ------------------------- BooleanConstant --------------------------

class BooleanConstant : public BooleanExpression
{
public:    
    typedef BooleanExpression NominalType;
    BooleanConstant( bool value );
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit ) const override;
    unique_ptr<BooleanResult> GetValue() const;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    BooleanResult::Matched matched;
};

};

#endif // include guard
