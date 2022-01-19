#include "expression.hpp"
#include "primary_expressions.hpp"

using namespace SYM;

// ------------------------- BooleanResult --------------------------

BooleanResult::BooleanResult( Category cat, bool value )
{
    switch( cat )
    {
    case UNDEFINED:
        ASSERT( !value );
        certainty = Certainty::UNDEFINED;
        break;
    case DEFINED:        
        certainty = value ? Certainty::TRUE : Certainty::FALSE;
        break;
    default:
        ASSERTFAIL("Missing case");
    }
}


bool BooleanResult::IsDefinedAndUnique() const
{
    return certainty != Certainty::UNDEFINED;
}


bool BooleanResult::IsDefinedAndTrue() const
{
    return certainty == Certainty::TRUE;
}


bool BooleanResult::IsDefinedAndFalse() const
{
    return certainty == Certainty::FALSE;
}

   
bool BooleanResult::GetAsBool() const
{
    ASSERT( IsDefinedAndUnique() );
    return certainty == Certainty::TRUE;
}


bool BooleanResult::operator<( const BooleanResult &other ) const
{
    return certainty < other.certainty;
}


bool BooleanResult::CertaintyCompare( const shared_ptr<BooleanResult> &a, 
                                      const shared_ptr<BooleanResult> &b )
{
    return *a < *b;
}                                      

// ------------------------- SymbolResult --------------------------

SymbolResult::SymbolResult( Category cat_, SR::XLink xlink_ ) :
    cat( cat_ ),
    xlink( xlink_ )
{
    switch( cat )
    {
    case UNDEFINED:
        ASSERT( !xlink );
        break;
    case DEFINED:
        ASSERT( xlink );
        break;
    default:
        ASSERTFAIL("Missing case");
    }
}


bool SymbolResult::IsDefinedAndUnique() const
{
    return cat != UNDEFINED;
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


bool Expression::IsIndependentOf( shared_ptr<Expression> target ) const
{
    if( OrderCompare( this, target.get() ) == EQUAL )
        return false; // we match the target, so not independent.
        
    for( shared_ptr<Expression> op : GetOperands() )
    {
        if( !op->IsIndependentOf( target ) )
            return false; // an operand may not be, so we may not be.
    }

    return true; // all operands are independent of target.
}


Orderable::Result Expression::OrderCompare( shared_ptr<const Expression> l, 
                                            shared_ptr<const Expression> r, 
                                            OrderProperty order_property )
{
    return OrderCompare( l.get(), r.get(), order_property );
}


Orderable::Result Expression::OrderCompareChildren( const Orderable *candidate, 
                                                    OrderProperty order_property ) const 
{
    ASSERT( candidate );
    auto *c = dynamic_cast<const Expression *>(candidate);    
    ASSERT(c);
    
    list<shared_ptr<Expression>> ll = GetOperands();
    list<shared_ptr<Expression>> rl = c->GetOperands();
    
    for( auto p : Zip(ll, rl) )     
    {
        Orderable::Result cr = OrderCompare( p.first, p.second, order_property );
        if( cr != EQUAL )
            return cr;
    }
    return EQUAL;
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

shared_ptr<Expression> BooleanExpression::TrySolveForToEqual( shared_ptr<Expression> target, 
                                                              shared_ptr<BooleanExpression> to_equal ) const
{
    // Make sure any solution is independent of target
    if( !to_equal->IsIndependentOf( target ) )
        return nullptr;
        
    // To solve: (this given target) == to_equal
    // So, if this===target then trivial solution: 
    // target==to_equal and to_equal is solution wrt target
    if( OrderCompare( this, target.get() ) == EQUAL )
        return to_equal;
    
    // Well that didn't work, try for non-trivial solutions
    return TrySolveForToEqualNT( target, to_equal );
}


shared_ptr<Expression> BooleanExpression::TrySolveForToEqualNT( shared_ptr<Expression> target, 
                                                                shared_ptr<BooleanExpression> to_equal ) const
{
    return nullptr;
}


BooleanExpression::PartialSolution BooleanExpression::PartialSolveFor( shared_ptr<Expression> target ) const
{
    // Could implement this in terms of TrySolveForToEqual() but since BooleanOperator::TrySolveFor()
    // calls PartialSolveFor() we could recurse indefinitely. So instead the rule is that
    // callers must try and use TrySolveForToEqual() before calling PartialSolveFor()
    return PartialSolution();
}

// ------------------------- SymbolExpression --------------------------

shared_ptr<Expression> SymbolExpression::TrySolveForToEqual( shared_ptr<Expression> target, 
                                                             shared_ptr<SymbolExpression> to_equal ) const
{
    // Make sure any solution is independent of target
    if( !to_equal->IsIndependentOf( target ) )
        return nullptr;
        
    // To solve: (this given target) == to_equal
    // So, if this===target then trivial solution: 
    // target==to_equal and to_equal is solution wrt target
    if( OrderCompare( this, target.get() ) == EQUAL )
        return to_equal;
    
    // Well that didn't work, try for non-trivial solutions
    return TrySolveForToEqualNT( target, to_equal );
}                                                             

shared_ptr<Expression> SymbolExpression::TrySolveForToEqualNT( shared_ptr<Expression> target, 
                                                               shared_ptr<SymbolExpression> to_equal ) const
{
    return nullptr;
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
