#include "expression.hpp"

using namespace SYM;

// ------------------------- BooleanResult --------------------------

BooleanResult::BooleanResult( Matched matched_ ) :
    matched( matched_ )
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

// ------------------------- BooleanExpression --------------------------

list<shared_ptr<BooleanExpression>> BooleanExpression::GetBooleanOperands() const
{
    return {};
}


list<shared_ptr<Expression>> BooleanExpression::GetOperands() const
{
    list<shared_ptr<Expression>> ops;
    for( shared_ptr<BooleanExpression> op : GetBooleanOperands() )
        ops.push_back( op );
    return ops;
}


unique_ptr<BooleanResult> BooleanExpression::Evaluate( const EvalKit &kit ) const
{ 
    list<unique_ptr<BooleanResult>> op_results;
    for( shared_ptr<BooleanExpression> a : GetBooleanOperands() )
        op_results.push_back( a->Evaluate(kit) );
    return Evaluate( kit, op_results );
    ASSERTFAIL("nope");
}


unique_ptr<BooleanResult> BooleanExpression::Evaluate( const EvalKit &kit, 
                                                       const list<unique_ptr<BooleanResult>> &op_results ) const
{
    ASSERTFAIL("Need to override one of the Evaluate() methods\n");
}

// ------------------------- SymbolExpression --------------------------

list<shared_ptr<SymbolExpression>> SymbolExpression::GetSymbolOperands() const
{
    return {};
}


list<shared_ptr<Expression>> SymbolExpression::GetOperands() const
{
    list<shared_ptr<Expression>> ops;
    for( shared_ptr<SymbolExpression> op : GetSymbolOperands() )
        ops.push_back( op );
    return ops;
}


unique_ptr<SymbolResult> SymbolExpression::Evaluate( const EvalKit &kit ) const
{ 
    list<unique_ptr<SymbolResult>> op_results;
    for( shared_ptr<SymbolExpression> a : GetSymbolOperands() )
        op_results.push_back( a->Evaluate(kit) );
    return Evaluate( kit, op_results );
}


unique_ptr<SymbolResult> SymbolExpression::Evaluate( const EvalKit &kit, 
                                                     const list<unique_ptr<SymbolResult>> &op_results ) const
{
    ASSERTFAIL("Need to override one of the Evaluate() methods\n");
}

// ------------------------- SymbolToBooleanExpression --------------------------

list<shared_ptr<Expression>> SymbolToBooleanExpression::GetOperands() const
{
    list<shared_ptr<Expression>> ops;
    for( shared_ptr<SymbolExpression> op : GetSymbolOperands() )
        ops.push_back( op );
    return ops;
}


unique_ptr<BooleanResult> SymbolToBooleanExpression::Evaluate( const EvalKit &kit ) const
{ 
    list<unique_ptr<SymbolResult>> op_results;
    for( shared_ptr<SymbolExpression> a : GetSymbolOperands() )
        op_results.push_back( a->Evaluate(kit) );
    return Evaluate( kit, op_results );
}


unique_ptr<BooleanResult> SymbolToBooleanExpression::Evaluate( const EvalKit &kit, 
                                                     const list<unique_ptr<SymbolResult>> &op_results ) const
{
    ASSERTFAIL("Need to override one of the Evaluate() methods\n");
}
