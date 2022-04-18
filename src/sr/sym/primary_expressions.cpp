#include "primary_expressions.hpp"
#include "result.hpp"

using namespace SYM;

// ------------------------- SymbolConstant --------------------------

SymbolConstant::SymbolConstant( SR::XLink xlink_ ) :
    xlink( xlink_ )
{
}


shared_ptr<SymbolResultInterface> SymbolConstant::Evaluate( const EvalKit &kit ) const
{
    return make_shared<SymbolResult>( SymbolResultInterface::DEFINED, xlink );
}


shared_ptr<SymbolResultInterface> SymbolConstant::GetValue() const
{
    return make_shared<SymbolResult>( SymbolResultInterface::DEFINED, xlink );
}


SR::XLink SymbolConstant::GetAsXLink() const
{
    return xlink;
}


Orderable::Result SymbolConstant::OrderCompareLocal( const Orderable *candidate, 
                                                     OrderProperty order_property ) const 
{
    auto c = GET_THAT_POINTER(candidate);

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


shared_ptr<SymbolResultInterface> SymbolVariable::Evaluate( const EvalKit &kit ) const
{
    // This is an ERROR. You could perfectly easily have called GetRequiredVariables(),
    // done a quick set difference and KNOWN that it would come to this.
    ASSERT( kit.hypothesis_links->count(plink) > 0 );
    
    return make_shared<SymbolResult>( SymbolResultInterface::DEFINED, kit.hypothesis_links->at(plink) );
}


SR::PatternLink SymbolVariable::GetPatternLink() const
{
    return plink;
}


Orderable::Result SymbolVariable::OrderCompareLocal( const Orderable *candidate, 
                                                     OrderProperty order_property ) const 
{
    auto c = GET_THAT_POINTER(candidate);

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
    value( value_ )
{
}


shared_ptr<BooleanResultInterface> BooleanConstant::Evaluate( const EvalKit &kit ) const
{
    return make_shared<BooleanResult>( value );
}


bool BooleanConstant::GetAsBool() const
{
    return value;
}


Orderable::Result BooleanConstant::OrderCompareLocal( const Orderable *candidate, 
                                                      OrderProperty order_property ) const 
{
    auto c = GET_THAT_POINTER(candidate);

    return value - c->value;
}  


string BooleanConstant::Render() const
{
    return value ? "TRUE" : "FALSE";
}


Expression::Precedence BooleanConstant::GetPrecedence() const
{
    return Precedence::LITERAL;
}

