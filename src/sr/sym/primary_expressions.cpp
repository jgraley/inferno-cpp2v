#include "primary_expressions.hpp"

using namespace SYM;

// ------------------------- SymbolConstant --------------------------

SymbolConstant::SymbolConstant( SR::XLink xlink_ ) :
    xlink( xlink_ )
{
}


unique_ptr<SymbolResult> SymbolConstant::Evaluate( const EvalKit &kit ) const
{
    return make_unique<SymbolResult>( xlink );
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

SymbolVariable::SymbolVariable( const SR::PatternLink &plink_ ) :
    plink( plink_ )
{
}


set<SR::PatternLink> SymbolVariable::GetRequiredVariables() const
{
    return { plink };
}


unique_ptr<SymbolResult> SymbolVariable::Evaluate( const EvalKit &kit ) const
{
    if( kit.hypothesis_links->count(plink) == 0 )
        return make_unique<SymbolResult>( SR::XLink() );
    else
        return make_unique<SymbolResult>( kit.hypothesis_links->at(plink) );
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


unique_ptr<BooleanResult> BooleanConstant::Evaluate( const EvalKit &kit ) const
{
    return make_unique<BooleanResult>( matched );
}


unique_ptr<BooleanResult> BooleanConstant::GetValue() const
{
    return make_unique<BooleanResult>( matched );
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

