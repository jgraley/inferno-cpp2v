#include "primary_expressions.hpp"

using namespace SYM;

// ------------------------- SymbolConstant --------------------------

SymbolConstant::SymbolConstant( SR::XLink xlink_ ) :
    xlink( xlink_ )
{
}


shared_ptr<SymbolResult> SymbolConstant::Evaluate( const EvalKit &kit ) const
{
    return make_shared<SymbolResult>( SymbolResult::DEFINED, xlink );
}


shared_ptr<SymbolResult> SymbolConstant::GetValue() const
{
    return make_shared<SymbolResult>( SymbolResult::DEFINED, xlink );
}


SR::XLink SymbolConstant::GetXLink() const
{
    return xlink;
}


Orderable::Result SymbolConstant::OrderCompareLocal( const Orderable *candidate, 
                                                     OrderProperty order_property ) const 
{
    ASSERT( candidate );
    auto *c = dynamic_cast<const SymbolConstant *>(candidate);    
    ASSERT(c);

    if( xlink == c->xlink )
        return 0;
    else if( xlink < c->xlink )
        return -1;
    else
        return 1;
}  


string SymbolConstant::Render() const
{
    return xlink.GetName();
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
        return make_shared<SymbolResult>( SymbolResult::DEFINED, kit.hypothesis_links->at(plink) );
}


SR::PatternLink SymbolVariable::GetPatternLink() const
{
    return plink;
}


Orderable::Result SymbolVariable::OrderCompareLocal( const Orderable *candidate, 
                                                     OrderProperty order_property ) const 
{
    ASSERT( candidate );
    auto *c = dynamic_cast<const SymbolVariable *>(candidate);    
    ASSERT(c);

    if( plink == c->plink )
        return 0;
    else if( plink < c->plink )
        return -1;
    else
        return 1;
}  


string SymbolVariable::Render() const
{
    return "[" + plink.GetShortName() + "]";
}


Expression::Precedence SymbolVariable::GetPrecedence() const
{
    return Precedence::LITERAL;
}

// ------------------------- BooleanConstant --------------------------

BooleanConstant::BooleanConstant( bool value_ ) :
    value( value_ ? BooleanResult::Certainty::TRUE : BooleanResult::Certainty::FALSE )
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


Orderable::Result BooleanConstant::OrderCompareLocal( const Orderable *candidate, 
                                                      OrderProperty order_property ) const 
{
    ASSERT( candidate );
    auto *c = dynamic_cast<const BooleanConstant *>(candidate);    
    ASSERT(c);

    return (int)value - (int)(c->value);
}  


string BooleanConstant::Render() const
{
    switch( value )
    {
    case BooleanResult::Certainty::TRUE:
        return "TRUE";
    case BooleanResult::Certainty::UNDEFINED:
        return "UNDEFINED";
    case BooleanResult::Certainty::FALSE:
        return "FALSE";
    default:
        ASSERTFAIL("Unknown boolean value");
    }    
}


Expression::Precedence BooleanConstant::GetPrecedence() const
{
    return Precedence::LITERAL;
}

