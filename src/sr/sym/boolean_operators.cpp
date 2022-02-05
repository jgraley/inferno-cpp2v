#include "boolean_operators.hpp"
#include "comparison_operators.hpp"
#include "symbol_operators.hpp"
#include "primary_expressions.hpp"
#include "rewriters.hpp"
#include <algorithm>

using namespace SYM;

// ------------------------- NotOperator --------------------------

NotOperator::NotOperator( shared_ptr<BooleanExpression> a_ ) :
    a( a_ )
{   
}    


list<shared_ptr<BooleanExpression>> NotOperator::GetBooleanOperands() const
{
    return { a };
}


shared_ptr<BooleanResult> NotOperator::Evaluate( const EvalKit &kit,
                                                 const list<shared_ptr<BooleanResult>> &op_results ) const
{
    shared_ptr<BooleanResult> ra = op_results.front();
    switch( ra->value )
    {   
    case BooleanResult::FALSE:
        return make_shared<BooleanResult>( BooleanResult::TRUE );
    case BooleanResult::UNDEFINED:
        return ra;
    case BooleanResult::TRUE:
        return make_shared<BooleanResult>( BooleanResult::FALSE );
    default:
        ASSERTFAIL("Missing case")
    }    
}


string NotOperator::Render() const
{
    return "~"+RenderForMe(a);
}


Expression::Precedence NotOperator::GetPrecedence() const
{
    return Precedence::PREFIX;
}


Over<BooleanExpression> SYM::operator~( Over<BooleanExpression> a )
{
    return MakeOver<NotOperator>( a );
}

// ------------------------- AndOperator --------------------------

AndOperator::AndOperator( list< shared_ptr<BooleanExpression> > sa_ ) :
    sa( sa_ )
{
}    


list<shared_ptr<BooleanExpression>> AndOperator::GetBooleanOperands() const
{
    return sa;
}


shared_ptr<BooleanResult> AndOperator::Evaluate( const EvalKit &kit,
                                                 const list<shared_ptr<BooleanResult>> &op_results ) const
{
    BooleanResult::BooleanValue m = BooleanResult::TRUE;
    for( const shared_ptr<BooleanResult> &r : op_results )    
        m = min( m, r->value );
        
    return make_shared<BooleanResult>( m );
}


shared_ptr<SymbolExpression> AndOperator::TrySolveFor( shared_ptr<SymbolVariable> target ) const
{
    set<shared_ptr<ImplicationOperator>> implies;
    set<shared_ptr<BoolEqualOperator>> bequals;
    map<shared_ptr<BooleanExpression>, shared_ptr<SymbolExpression>> solveables;
    for( shared_ptr<BooleanExpression> op : sa )
    {
        if( auto o = dynamic_pointer_cast<ImplicationOperator>(op) )       
            implies.insert(o);
        if( auto o = dynamic_pointer_cast<BoolEqualOperator>(op) )       
            bequals.insert(o);
        if( shared_ptr<SymbolExpression> solved = op->TrySolveFor( target ) )
            solveables[op] = solved;
    }
    
    // Standard algorithm - first thing to try is to see if any of the clauses provide a solution
    if( !solveables.empty() )
        return FrontOf(solveables).second;

    // TODO:
    // Build an implication table, mapping bool x expr -> set<expr>
    // (or multimap). The bools are "not" flags (NotOperator to be detected and removed, toggling flag)
    // BooleanEqual becomes 2 entries in table: left and right
    // To solve, search for (true, a)->b and (false, a)->c in the table SUCH THAT
    // a indep t; b and c solveable for t.
    
    // "Special Stuff" for solving the standard clutch logic (EQUALITY_METHOD only)
    if( implies.size()==1 && 
        bequals.size()==1 )
    {
        list< shared_ptr<BooleanExpression> > imply_ops = OnlyElementOf(implies)->GetBooleanOperands();           
        list< shared_ptr<BooleanExpression> > beq_ops = OnlyElementOf(bequals)->GetBooleanOperands(); 
        
        shared_ptr<EqualOperator> imply_nequal_op;
        shared_ptr<EqualOperator> beq_equal_op;        
        if( auto imply_not_op = dynamic_pointer_cast<NotOperator>(imply_ops.front()) )
            imply_nequal_op = dynamic_pointer_cast<EqualOperator>(imply_not_op->GetOperands().front());
        beq_equal_op = dynamic_pointer_cast<EqualOperator>(beq_ops.front()); 
                                  
        if( imply_nequal_op && beq_equal_op && 
            Expression::OrderCompare( imply_nequal_op, beq_equal_op ) == EQUAL )
        {
            // Fronts of imply and beq are negation of each other.
            shared_ptr<SymbolExpression> try_solve_b = beq_ops.back()->TrySolveFor(target);
            shared_ptr<SymbolExpression> try_solve_c = imply_ops.back()->TrySolveFor(target);
            if( try_solve_b && try_solve_c && beq_ops.front()->IsIndependentOf(target))
            {
                return make_shared<ConditionalOperator>( beq_ops.front(),
                                                         try_solve_b,
                                                         try_solve_c );
            }
        }
    }    
    return nullptr;
}


string AndOperator::Render() const
{
    if( sa.empty() )
        return "TRUE(empty-And)";
    list<string> ls;
    for( shared_ptr<BooleanExpression> a : sa )
        ls.push_back( RenderForMe(a) );
    return Join( ls, " & " );
}


Expression::Precedence AndOperator::GetPrecedence() const
{
    return Precedence::AND;
}


Over<BooleanExpression> SYM::operator&( Over<BooleanExpression> a, Over<BooleanExpression> b )
{
    // Overloaded operator can only take 2 args, but operator is commutative and
    // associative: we want a o b o c to generate Operator({a, b, c}) not
    // some nested pair. Note: this can over-kill but I don't expect that to cause
    // problems.
    return CreateTidiedOperator<AndOperator>(true)({ a, b });
}

// ------------------------- OrOperator --------------------------

OrOperator::OrOperator( list< shared_ptr<BooleanExpression> > sa_ ) :
    sa( sa_ )
{   
}    


list<shared_ptr<BooleanExpression>> OrOperator::GetBooleanOperands() const
{
    return sa;
}


shared_ptr<BooleanResult> OrOperator::Evaluate( const EvalKit &kit,
                                                const list<shared_ptr<BooleanResult>> &op_results ) const
{
    BooleanResult::BooleanValue m = BooleanResult::FALSE;
    for( const shared_ptr<BooleanResult> &r : op_results )
        m = max( m, r->value );

    return make_shared<BooleanResult>( m );
}


string OrOperator::Render() const
{
    if( sa.empty() )
        return "FALSE(empty-Or)";
    list<string> ls;
    for( shared_ptr<BooleanExpression> a : sa )
        ls.push_back( RenderForMe(a) );
    return Join( ls, " | " );
}


Expression::Precedence OrOperator::GetPrecedence() const
{
    return Precedence::OR;
}


Over<BooleanExpression> SYM::operator|( Over<BooleanExpression> a, Over<BooleanExpression> b )
{
    // Overloaded operator can only take 2 args, but operator is commutative and
    // associative: we want a o b o c to generate Operator({a, b, c}) not
    // some nested pair. Note: this can over-kill but I don't expect that to cause
    // problems.
    return CreateTidiedOperator<OrOperator>(false)({ a, b });
}


// ------------------------- BoolEqualOperator --------------------------

BoolEqualOperator::BoolEqualOperator( shared_ptr<BooleanExpression> a_, 
                                      shared_ptr<BooleanExpression> b_ ) :
    a( a_ ),
    b( b_ )
{   
}    


list<shared_ptr<BooleanExpression>> BoolEqualOperator::GetBooleanOperands() const
{
    return {a, b};
}


shared_ptr<BooleanResult> BoolEqualOperator::Evaluate( const EvalKit &kit,
                                                       const list<shared_ptr<BooleanResult>> &op_results ) const
{
    shared_ptr<BooleanResult> ra = op_results.front();
    shared_ptr<BooleanResult> rb = op_results.back();
    
    if( ra->value == BooleanResult::UNDEFINED )
        return ra;
        
    if( rb->value == BooleanResult::UNDEFINED )
        return rb;
    
    if( ra->value == rb->value )
        return make_shared<BooleanResult>( BooleanResult::TRUE );
    else
        return make_shared<BooleanResult>( BooleanResult::FALSE );     
}


string BoolEqualOperator::Render() const
{
    return RenderForMe(a) + " == " + RenderForMe(b);
}


Expression::Precedence BoolEqualOperator::GetPrecedence() const
{
    return Precedence::COMPARE;
}


Over<BooleanExpression> SYM::operator==( Over<BooleanExpression> a, Over<BooleanExpression> b )
{
    return MakeOver<BoolEqualOperator>( a, b );
}

// ------------------------- ImplicationOperator --------------------------

ImplicationOperator::ImplicationOperator( shared_ptr<BooleanExpression> a_,
                                          shared_ptr<BooleanExpression> b_ ) :
    a( a_ ),
    b( b_ )
{   
}    


list<shared_ptr<BooleanExpression>> ImplicationOperator::GetBooleanOperands() const
{
    return { a, b };
}


shared_ptr<BooleanResult> ImplicationOperator::Evaluate( const EvalKit &kit,
                                                         const list<shared_ptr<BooleanResult>> &op_results ) const
{
    shared_ptr<BooleanResult> ra = op_results.front();
    shared_ptr<BooleanResult> rb = op_results.back();
    switch( ra->value )
    {   
    case BooleanResult::FALSE:
        return make_shared<BooleanResult>( BooleanResult::TRUE );
    case BooleanResult::UNDEFINED:
        if( rb->value == BooleanResult::TRUE )
            return rb;
        else
            return ra;
    case BooleanResult::TRUE:
        return rb;
    default:
        ASSERTFAIL("Missing case")
    }  
}


string ImplicationOperator::Render() const
{
    return RenderForMe(a)+" ⇒ "+RenderForMe(b);
}


Expression::Precedence ImplicationOperator::GetPrecedence() const
{
    return Precedence::IMPLICATION;
}
