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
    explicit SymbolConstant( SR::XLink xlink );
    virtual shared_ptr<SymbolResult> Evaluate( const EvalKit &kit ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const SR::XLink xlink;
};

// ------------------------- SymbolVariable --------------------------

class SymbolVariable : public SymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    explicit SymbolVariable( const SR::PatternLink &plink );
    virtual set<SR::PatternLink> GetRequiredVariables() const override;
    virtual shared_ptr<SymbolResult> Evaluate( const EvalKit &kit ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const SR::PatternLink plink;
};

// ------------------------- BooleanConstant --------------------------

class BooleanConstant : public BooleanExpression
{
public:    
    typedef BooleanExpression NominalType;
    explicit BooleanConstant( bool value );
    virtual shared_ptr<BooleanResult> Evaluate( const EvalKit &kit ) const override;
    shared_ptr<BooleanResult> GetValue() const;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const BooleanResult::BooleanValue value;
};

};

#endif // include guard
