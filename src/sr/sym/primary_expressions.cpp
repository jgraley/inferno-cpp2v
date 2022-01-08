#include "primary_expressions.hpp"

using namespace SYM;

// ------------------------- SymbolConstant --------------------------

SymbolConstant::SymbolConstant( SR::XLink xlink_ ) :
    xlink( xlink_ )
{
}


shared_ptr<SymbolResult> SymbolConstant::Evaluate( const EvalKit &kit ) const
{
    return make_shared<SymbolResult>( SymbolResult::XLINK, xlink );
}


shared_ptr<SymbolResult> SymbolConstant::GetValue() const
{
    return make_shared<SymbolResult>( SymbolResult::XLINK, xlink );
}


SR::XLink SymbolConstant::GetXLink() const
{
    return xlink;
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


shared_ptr<SymbolResult> SymbolVariable::Evaluate( const EvalKit &kit ) const
{
    if( kit.hypothesis_links->count(plink) == 0 )
        return make_shared<SymbolResult>( SymbolResult::UNDEFINED );
    else
        return make_shared<SymbolResult>( SymbolResult::XLINK, kit.hypothesis_links->at(plink) );
}


bool SymbolVariable::IsIndependentOf( shared_ptr<SymbolVariable> target ) const
{
    return GetPatternLink() != target->GetPatternLink();      
}


SR::PatternLink SymbolVariable::GetPatternLink() const
{
    return plink;
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
    value( value_ ? BooleanResult::TRUE : BooleanResult::FALSE )
{
}


shared_ptr<BooleanResult> BooleanConstant::Evaluate( const EvalKit &kit ) const
{
    return make_shared<BooleanResult>( value );
}


shared_ptr<BooleanResult> BooleanConstant::GetValue() const
{
    return make_shared<BooleanResult>( value );
}


string BooleanConstant::Render() const
{
    switch( value )
    {
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

