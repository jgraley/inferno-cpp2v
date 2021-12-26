#include "primary_expressions.hpp"

using namespace SYM;

//#define THROW_ON_INCOMPLETE

// ------------------------- SymbolConstant --------------------------

SymbolConstant::SymbolConstant( SR::XLink xlink_ ) :
    xlink( xlink_ )
{
}


shared_ptr<SymbolResult> SymbolConstant::Evaluate( const EvalKit &kit ) const
{
    return make_shared<SymbolResult>( xlink );
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
#ifdef THROW_ON_INCOMPLETE
        throw Incomplete(plink);
#else
        return make_shared<SymbolResult>( SR::XLink() );
#endif        
    
    return make_shared<SymbolResult>( kit.hypothesis_links->at(plink) );
}


string SymbolVariable::Render() const
{
    return "[" + plink.GetTrace() + "]";
}


Expression::Precedence SymbolVariable::GetPrecedence() const
{
    return Precedence::LITERAL;
}


SymbolVariable::Incomplete::Incomplete( const SR::PatternLink &plink_ ) :
    plink( plink_ )
{
}


string SymbolVariable::Incomplete::What() const noexcept
{
    return Exception::What() + "(" + plink.GetTrace() + ")";
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

