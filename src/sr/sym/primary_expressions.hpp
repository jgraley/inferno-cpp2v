#ifndef PRIMARY_EXPRESSIONS_HPP
#define PRIMARY_EXPRESSIONS_HPP

#include "expression.hpp"
#include "overloads.hpp"

#include "common/mismatch.hpp"
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
    virtual shared_ptr<SymbolResultInterface> Evaluate( const EvalKit &kit ) const override;    
    shared_ptr<SymbolResultInterface> GetValue() const;
    SR::XLink GetAsXLink() const;

    virtual Orderable::Result OrderCompareLocal( const Orderable *candidate, 
                                                 OrderProperty order_property ) const override;

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
    virtual shared_ptr<SymbolResultInterface> Evaluate( const EvalKit &kit ) const override;
    SR::PatternLink GetPatternLink() const;

    // Note: no TrySolveForToEqualNT() because trivial solver is sufficient
    Orderable::Result OrderCompareLocal( const Orderable *candidate, 
                                         OrderProperty order_property ) const override;

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
    virtual shared_ptr<BooleanResultInterface> Evaluate( const EvalKit &kit ) const override;
    bool GetAsBool() const;    

    virtual Orderable::Result OrderCompareLocal( const Orderable *candidate, 
                                                 OrderProperty order_property ) const override;

    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const bool value;
};

};

#endif // include guard
