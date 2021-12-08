#include "primary_expressions.hpp"

using namespace SYM;

// ------------------------- Constant --------------------------

Constant::Constant( SR::XLink xlink_ ) :
    xlink( xlink_ )
{
}


SymbolResult Constant::Evaluate( const EvalKit &kit ) const
{
    return { xlink };
}


string Constant::Render() const
{
    return xlink.GetTrace();
}


Expression::Precedence Constant::GetPrecedence() const
{
    return Precedence::LITERAL;
}

// ------------------------- Variable --------------------------

Variable::Variable( SR::PatternLink plink_ ) :
    plink( plink_ )
{
}


set<SR::PatternLink> Variable::GetRequiredPatternLinks() const
{
    return { plink };
}


SymbolResult Variable::Evaluate( const EvalKit &kit ) const
{
    return { kit.hypothesis_links->at(plink) };
}


string Variable::Render() const
{
    return "[" + plink.GetTrace() + "]";
}


Expression::Precedence Variable::GetPrecedence() const
{
    return Precedence::LITERAL;
}
