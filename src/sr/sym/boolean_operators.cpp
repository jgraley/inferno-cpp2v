#include "boolean_operators.hpp"
#include "comparison_operators.hpp"
#include "symbol_operators.hpp"
#include "primary_expressions.hpp"
#include "rewriters.hpp"

using namespace SYM;

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
    {
        switch( r->value )
        {            
        case BooleanResult::TRUE:
            break;
        case BooleanResult::FALSE:
            return make_shared<BooleanResult>( BooleanResult::FALSE );
        }
    }
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
    
    // "Special Stuff" for solving the standard clutch logic (EQUALITY_METHOD only)
    if( implies.size()==1 && 
        bequals.size()==1 )
    {
        list< shared_ptr<BooleanExpression> > imply_ops = OnlyElementOf(implies)->GetBooleanOperands();           
        list< shared_ptr<BooleanExpression> > beq_ops = OnlyElementOf(bequals)->GetBooleanOperands(); 
        
        // Ugly bit
        SR::PatternLink beq_plink, imply_plink;
        SR::XLink beq_xlink, imply_xlink;
        if( auto imply_nequal_op = dynamic_pointer_cast<NotEqualOperator>(imply_ops.front() ) )
        {
            if( auto imply_nequal_lop = dynamic_pointer_cast<SymbolVariable>(imply_nequal_op->GetOperands().front() ) )
                imply_plink = imply_nequal_lop->GetPatternLink();
            if( auto imply_nequal_rop = dynamic_pointer_cast<SymbolConstant>(imply_nequal_op->GetOperands().back() ) )
                imply_xlink = imply_nequal_rop->GetXLink();
        }
        if( auto beq_equal_op = dynamic_pointer_cast<EqualOperator>(beq_ops.front() ) )
        {
            if( auto beq_equal_lop = dynamic_pointer_cast<SymbolVariable>(beq_equal_op->GetOperands().front() ) )
                beq_plink = beq_equal_lop->GetPatternLink();
            if( auto beq_equal_rop = dynamic_pointer_cast<SymbolConstant>(beq_equal_op->GetOperands().back() ) )
                beq_xlink = beq_equal_rop->GetXLink();
        }            

        if( beq_plink && beq_plink==imply_plink &&
            beq_xlink && beq_xlink==imply_xlink )
        {
            // Fronts of imply and beq are negation of each other.
            shared_ptr<SymbolExpression> try_solve_b = beq_ops.back()->TrySolveFor(target);
            shared_ptr<SymbolExpression> try_solve_c = imply_ops.back()->TrySolveFor(target);
            if( try_solve_b && try_solve_c )
            {
                TRACE("Solved an and!!\n");
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
    {
        switch( r->value )
        {   
        case BooleanResult::TRUE:
            return make_shared<BooleanResult>( BooleanResult::TRUE );
        case BooleanResult::FALSE:
            break;
        }
    }
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
    if( ra->value == BooleanResult::TRUE )
        return rb;
    else 
        return make_shared<BooleanResult>( BooleanResult::TRUE );
}


string ImplicationOperator::Render() const
{
    return RenderForMe(a)+" ⇒ "+RenderForMe(b);
}


Expression::Precedence ImplicationOperator::GetPrecedence() const
{
    return Precedence::IMPLICATION;
}
