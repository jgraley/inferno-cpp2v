#include "expression.hpp"

using namespace SYM;

list<shared_ptr<Expression>> Expression::GetOperands() const
{
    return {};
}


set<SR::PatternLink> Expression::GetRequiredVariables() const 
{
    set<SR::PatternLink> required_vars;
    // Non-strict union (i.e. not Solo) because common links are fine
    for( const shared_ptr<Expression> a : GetOperands() )
        required_vars = UnionOf( required_vars, a->GetRequiredVariables() );
    return required_vars;
}


string Expression::RenderForMe( shared_ptr<const Expression> inner ) const
{
    string bare = inner->Render();
    Precedence inner_prec = inner->GetPrecedence();
    Precedence me_prec = GetPrecedence();
    
    // Bigger number is LOWER precedence. Parents required when putting
    // low prec expr in high prec surroundings. Also when equal to avoid
    // assuming associativity.
    if( (int)inner_prec >= (int)me_prec )
        return "(" + bare + ")";
    else
        return bare;    
}


string Expression::GetTrace() const
{
    return "SYM::\"" + 
           Render() + 
           "\" requiring " + 
           Trace(GetRequiredVariables());
}


BooleanResult::BooleanResult( Matched matched_ ) :
    matched( matched_ )
{
}


SymbolResult::SymbolResult()
{
}

SymbolResult::SymbolResult( const SR::XLink &xlink_ ) :
    xlink( xlink_ )
{
}

