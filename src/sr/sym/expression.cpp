#include "expression.hpp"

using namespace SYM;

// ------------------------- BooleanResult --------------------------

BooleanResult::BooleanResult( BooleanValue value_ ) :
    value( value_ )
{
}

// ------------------------- SymbolResult --------------------------

SymbolResult::SymbolResult()
{
}

SymbolResult::SymbolResult( const SR::XLink &xlink_ ) :
    xlink( xlink_ )
{
}

// ------------------------- Expression --------------------------

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


shared_ptr<SymbolExpression> Expression::TrySolveFor( shared_ptr<SymbolVariable> target ) const
{
    return nullptr;
}


bool Expression::IsIndependentOf( shared_ptr<SymbolVariable> target ) const
{
    // When we extend to allow target to be any Expression, we'll want
    // do do an expression compare in here (which means we need expression 
    // compare) before the loop, and we'll remove the overload in 
    // SymbolVariable, and possibly remove the virtual from here.
    for( shared_ptr<Expression> op : GetOperands() )
    {
        if( !op->IsIndependentOf( target ) )
            return false; // an operand may not be, so we may not be.
    }

    return true; // all operands are independent of target
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

// ------------------------- BooleanToBooleanExpression --------------------------

list<shared_ptr<BooleanExpression>> BooleanToBooleanExpression::GetBooleanOperands() const
{
    return {};
}


list<shared_ptr<Expression>> BooleanToBooleanExpression::GetOperands() const
{
    list<shared_ptr<Expression>> ops;
    for( shared_ptr<BooleanExpression> op : GetBooleanOperands() )
        ops.push_back( op );
    return ops;
}


shared_ptr<BooleanResult> BooleanToBooleanExpression::Evaluate( const EvalKit &kit ) const
{ 
    list<shared_ptr<BooleanResult>> op_results;
    for( shared_ptr<BooleanExpression> a : GetBooleanOperands() )
        op_results.push_back( a->Evaluate(kit) );
    return Evaluate( kit, op_results );
}


shared_ptr<BooleanResult> BooleanToBooleanExpression::Evaluate( const EvalKit &kit, 
                                                                const list<shared_ptr<BooleanResult>> &op_results ) const
{
    ASSERTFAIL("Need to override one of the Evaluate() methods\n");
}


shared_ptr<SymbolResult> SymbolToSymbolExpression::Evaluate( const EvalKit &kit, 
                                                             const list<shared_ptr<SymbolResult>> &op_results ) const
{
    ASSERTFAIL("Need to override one of the Evaluate() methods\n");
}

// ------------------------- SymbolToSymbolExpression --------------------------

list<shared_ptr<SymbolExpression>> SymbolToSymbolExpression::GetSymbolOperands() const
{
    return {};
}


list<shared_ptr<Expression>> SymbolToSymbolExpression::GetOperands() const
{
    list<shared_ptr<Expression>> ops;
    for( shared_ptr<SymbolExpression> op : GetSymbolOperands() )
        ops.push_back( op );
    return ops;
}


shared_ptr<SymbolResult> SymbolToSymbolExpression::Evaluate( const EvalKit &kit ) const
{ 
    list<shared_ptr<SymbolResult>> op_results;
    for( shared_ptr<SymbolExpression> a : GetSymbolOperands() )
        op_results.push_back( a->Evaluate(kit) );
    return Evaluate( kit, op_results );
}

// ------------------------- SymbolToBooleanExpression --------------------------

list<shared_ptr<Expression>> SymbolToBooleanExpression::GetOperands() const
{
    list<shared_ptr<Expression>> ops;
    for( shared_ptr<SymbolExpression> op : GetSymbolOperands() )
        ops.push_back( op );
    return ops;
}


shared_ptr<BooleanResult> SymbolToBooleanExpression::Evaluate( const EvalKit &kit ) const
{ 
    list<shared_ptr<SymbolResult>> op_results;
    for( shared_ptr<SymbolExpression> a : GetSymbolOperands() )
        op_results.push_back( a->Evaluate(kit) );
    return Evaluate( kit, op_results );
}


shared_ptr<BooleanResult> SymbolToBooleanExpression::Evaluate( const EvalKit &kit, 
                                                               const list<shared_ptr<SymbolResult>> &op_results ) const
{
    ASSERTFAIL("Need to override one of the Evaluate() methods\n");
}
