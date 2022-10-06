#include "expression.hpp"
#include "result.hpp"
#include "symbol_operators.hpp"

using namespace SYM;

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


Expression::VariablesRequiringRows Expression::GetVariablesRequiringRows() const
{
    VariablesRequiringRows k;
    for( shared_ptr<Expression> p : GetOperands() )
        k = UnionOf( k, p->GetVariablesRequiringRows() );
    return k;
}


void Expression::ForDepthFirstWalk( function<void(const Expression *)> f ) const
{
    f( this );
    for( const shared_ptr<Expression> a : GetOperands() )
        a->ForDepthFirstWalk( f );
}


bool Expression::IsIndependentOf( shared_ptr<Expression> target ) const
{
    if( OrderCompareEqual( this, target.get() ) )
        return false; // we match the target, so not independent.
        
    for( shared_ptr<Expression> op : GetOperands() )
    {
        if( !op->IsIndependentOf( target ) )
            return false; // an operand may not be, so we may not be.
    }

    return true; // all operands are independent of target.
}


Orderable::Diff Expression::OrderCompare3Way( shared_ptr<const Expression> l, 
                                              shared_ptr<const Expression> r, 
                                              OrderProperty order_property )
{
    return OrderCompare3Way( l.get(), r.get(), order_property );
}


bool Expression::OrderCompareEqual( shared_ptr<const Expression> l, 
                                    shared_ptr<const Expression> r, 
                                    OrderProperty order_property )
{
    return OrderCompareEqual( l.get(), r.get(), order_property );
}


Orderable::Diff Expression::OrderCompare3WayChildren( const Orderable *candidate, 
                                                      OrderProperty order_property ) const 
{
    ASSERT( candidate );
    auto *c = dynamic_cast<const Expression *>(candidate);    
    ASSERT(c);
    
    list<shared_ptr<Expression>> ll = GetOperands();
    list<shared_ptr<Expression>> rl = c->GetOperands();
    
    if( IsCommutative() )
    {
        // Commutative
        set<shared_ptr<Expression>, Relation> lo;
        for( shared_ptr<Expression> e : ll )     
            lo.insert( e );

        set<shared_ptr<Expression>, Relation> ro;
        for( shared_ptr<Expression> e : rl )      
            ro.insert( e );

        return LexicographicalCompare( lo, ro, Relation() );
    }
    else
    {
        // Non-commutative
        return LexicographicalCompare( ll, rl, Relation() );
    }
}


Orderable::Diff Expression::Relation::Compare3Way( const shared_ptr<const Expression> &l, 
                                                   const shared_ptr<const Expression> &r ) const
{
    return Expression::OrderCompare3Way( l, r );
}                                                        


bool Expression::Relation::operator()( const shared_ptr<const Expression> &l, 
                                       const shared_ptr<const Expression> &r ) const
{
    return Compare3Way(l, r) < 0;
}                                      


bool Expression::IsCommutative() const
{
    return false;
}


string Expression::RenderWithParentheses() const
{
    return "(" + Render() + ")";
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
        return inner->RenderWithParentheses();
    else
        return inner->Render();    
}


string Expression::GetTrace() const
{
    return "SYM::\"" + 
           Render() + 
           "\" requiring " + 
           Trace(GetRequiredVariables());
}

// ------------------------- BooleanExpression --------------------------

shared_ptr<SymbolExpression> BooleanExpression::TrySolveFor( const SolveKit &kit, shared_ptr<SymbolVariable> target ) const
{
    return nullptr;
}

// ------------------------- SymbolExpression --------------------------

shared_ptr<SymbolExpression> SymbolExpression::TrySolveForToEqual( const SolveKit &kit, shared_ptr<SymbolVariable> target, 
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


unique_ptr<BooleanResult> BooleanToBooleanExpression::Evaluate( const EvalKit &kit ) const
{ 
    list<unique_ptr<BooleanResult>> op_results;
    for( shared_ptr<BooleanExpression> a : GetBooleanOperands() )
        op_results.push_back( a->Evaluate(kit) );
    return Evaluate( kit, move(op_results) );
}


unique_ptr<BooleanResult> BooleanToBooleanExpression::Evaluate( const EvalKit &kit, 
                                                                list<unique_ptr<BooleanResult>> &&op_results ) const
{
    ASSERTFAIL("Need to override one of the Evaluate() methods\n");
}


unique_ptr<SymbolResultInterface> SymbolToSymbolExpression::Evaluate( const EvalKit &kit, 
                                                             list<unique_ptr<SymbolResultInterface>> &&op_results ) const
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


unique_ptr<SymbolResultInterface> SymbolToSymbolExpression::Evaluate( const EvalKit &kit ) const
{ 
    list<unique_ptr<SymbolResultInterface>> op_results;
    for( shared_ptr<SymbolExpression> a : GetSymbolOperands() )
        op_results.push_back( a->Evaluate(kit) );
    return Evaluate( kit, move(op_results) );
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
    list<unique_ptr<SymbolResultInterface>> op_results;
    for( shared_ptr<SymbolExpression> a : GetSymbolOperands() )
        op_results.push_back( a->Evaluate(kit) );
    return Evaluate( kit, move(op_results) );
}


unique_ptr<BooleanResult> SymbolToBooleanExpression::Evaluate( const EvalKit &kit, 
                                                               list<unique_ptr<SymbolResultInterface>> &&op_results ) const
{
    ASSERTFAIL("Need to override one of the Evaluate() methods\n");
}
