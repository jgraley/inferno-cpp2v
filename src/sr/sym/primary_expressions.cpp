#include "primary_expressions.hpp"

using namespace SYM;

// ------------------------- SymbolConstant --------------------------

SymbolConstant::SymbolConstant( SR::XLink xlink_ ) :
    xlink( xlink_ )
{
}


SymbolResult SymbolConstant::Evaluate( const EvalKit &kit ) const
{
    return { xlink };
}


string SymbolConstant::Render() const
{
    return xlink.GetTrace();
}


Expression::Precedence SymbolConstant::GetPrecedence() const
{
    return Precedence::LITERAL;
}

// ------------------------- SymbolVariable --------------------------

SymbolVariable::SymbolVariable( SR::PatternLink plink_ ) :
    plink( plink_ )
{
}


set<SR::PatternLink> SymbolVariable::GetRequiredPatternLinks() const
{
    return { plink };
}


SymbolResult SymbolVariable::Evaluate( const EvalKit &kit ) const
{
    return { kit.hypothesis_links->at(plink) };
}


string SymbolVariable::Render() const
{
    return "[" + plink.GetTrace() + "]";
}


Expression::Precedence SymbolVariable::GetPrecedence() const
{
    return Precedence::LITERAL;
}

// ------------------------- BooleanConstant --------------------------

BooleanConstant::BooleanConstant( bool value_ ) :
    value( value_ )
{
}


BooleanResult BooleanConstant::Evaluate( const EvalKit &kit ) const
{
    return { value, nullptr };
}


string BooleanConstant::Render() const
{
    return value ? "true" : "false";
}


Expression::Precedence BooleanConstant::GetPrecedence() const
{
    return Precedence::LITERAL;
}

