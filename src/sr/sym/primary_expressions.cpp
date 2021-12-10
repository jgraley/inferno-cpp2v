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
    if( kit.hypothesis_links->count(plink) == 0 )
        return { SR::XLink() };
    else
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

BooleanConstant::BooleanConstant( bool value ) :
    matched( value ? BooleanResult::TRUE : BooleanResult::FALSE )
{
}


BooleanResult BooleanConstant::Evaluate( const EvalKit &kit ) const
{
    return { matched, nullptr };
}


string BooleanConstant::Render() const
{
    switch( matched )
    {
    case BooleanResult::UNKNOWN:
        return "UNKNOWN";
    case BooleanResult::TRUE:
        return "TRUE";
    case BooleanResult::FALSE:
        return "FALSE";
    }
    ASSERTFAIL("Invalid matched");
}


Expression::Precedence BooleanConstant::GetPrecedence() const
{
    return Precedence::LITERAL;
}

