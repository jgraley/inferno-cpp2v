#include "boolean_operators.hpp"
#include "comparison_operators.hpp"
#include "symbol_operators.hpp"
#include "primary_expressions.hpp"
#include "rewriters.hpp"
#include <algorithm>

using namespace SYM;

#define SOLVE_FROM_PARTIALS_CHECKING

// ------------------------- BooleanOperator --------------------------

shared_ptr<Expression> BooleanOperator::TrySolveFor( shared_ptr<Expression> target ) const
{
    INDENT("T");
    PartialSolution psol = PartialSolveFor( target );
    
#ifdef SOLVE_FROM_PARTIALS_CHECKING
    for( auto &ps : psol ) // all senses (true, false)
    {
        for( auto p : ps.second ) // all entries for the current sense
        {
            // NotOperators should have been removed from condition
            ASSERT( !dynamic_pointer_cast<NotOperator>( p.first ) );

            // Is a solution to a sub-expression so should not depend on target 
            ASSERT( p.second->IsIndependentOf( target ) );                    
        }
    }
#endif

    // Try to solve FOR our dependent keys and substitute the solution or remove
    for( auto &ps : psol ) // all senses (true, false)
    {        
//        FTRACEC("Before substitution ")(ps.first)(":\n")(ps.second)("\n");
        while( true )
        {
            shared_ptr<BooleanExpression> found_dependent_key;
            shared_ptr<Expression> found_dependent_value;
            for( auto p : ps.second ) // all entries for the current sense            
                if( !p.first->IsIndependentOf( target ) )                
                    tie( found_dependent_key, found_dependent_value ) = p;
            
            // HAve we managed to remove all dependent keys?
            if( !found_dependent_key )
                break;
                    
            // Now safe to erase
            ps.second.erase( found_dependent_key );

            // Try to solve this expression (it's the biggest we can see) with
            // respect to the dependent key sub-expression (i.e. a new target) 
            // in the hopes of getting hold of an independent sub-expression
            // that we can substitute for the key.
//            FTRACEC("Substitution: trying to solve:\n")(Render())
//                   ("\nwith respect to:\n")(found_dependent_key)("\n");
            shared_ptr<Expression> solution = TrySolveFor( found_dependent_key );
            
            if( !solution )    
            {
//                FTRACEC("but FAILED\n\n");
                continue;
            }
            
            auto solved_key = dynamic_pointer_cast<BooleanExpression>(solution);
            ASSERT(solved_key)("Definitely expected a BooleanExpression\n");
            
//            FTRACEC("and got:\n")(solved_key)("\n\n");
            // Note: does NOT guarantee indepedennce of target, since we solved 
            // wrt something else (found_dependent_key). However, the while()
            // loop won't exit until all keys are independent.
            ps.second[solved_key] = found_dependent_value;      
        }
//        FTRACEC("After substitution ")(ps.first)(":\n")(ps.second)("\n");
    }

    // Try to find an a=>b & !a=>c pair and solve using a conditional
    // TODO consider explicitly using ordering in the map (doesn't happen automatically
    // because shared_ptr<> is in the way) so we can "find" in negative sense partials.
    shared_ptr<BooleanExpression> cond, te, be;
    for( pair< shared_ptr<BooleanExpression>, 
               shared_ptr<Expression> > posp : psol[true] ) // all entries for positive sense
    {
        ASSERT( posp.first->IsIndependentOf( target ) ); // all dependent keys should have been removed above     
        for( pair< shared_ptr<BooleanExpression>, 
                   shared_ptr<Expression> > negp : psol[false] ) // all entries for negative sense
        {
            ASSERT( negp.first->IsIndependentOf( target ) ); // all dependent keys should have been removed above     
            if( OrderCompare( posp.first, negp.first ) == EQUAL ) // same expression
            {
                // If do #477, could simplify
                if( auto sym_pos = dynamic_pointer_cast<SymbolExpression>(posp.second) )
                    if( auto sym_neg = dynamic_pointer_cast<SymbolExpression>(negp.second) )
                        return make_shared<ConditionalOperator>( posp.first, sym_pos, sym_neg );
                if( auto bool_pos = dynamic_pointer_cast<BooleanExpression>(posp.second) )
                    if( auto bool_neg = dynamic_pointer_cast<BooleanExpression>(negp.second) )
                        return make_shared<BooleanConditionalOperator>( posp.first, bool_pos, bool_neg );
                // It's not an error if one is symbolic and the other boolean, but there 
                // is no solution in such a scenario.
            }
        }
    }
    return nullptr;
}


void BooleanOperator::TryAddPartialSolutionFor( shared_ptr<Expression> target,
                                                PartialSolution &psol, 
                                                bool key_sense, 
                                                shared_ptr<BooleanExpression> key, 
                                                shared_ptr<BooleanExpression> val_unsolved ) const
{
    INDENT("A");
//    FTRACEC("Should I add wrt target: ")(target)
//           ("\nkey: ")(key)
//           ("\nvalue unsolved: ")(val_unsolved)("\n");
           
    // Try solving value for target
    shared_ptr<Expression> val = val_unsolved->TrySolveFor(target);
    if( !val )
    {
//        FTRACE("No, could not solve\n");
        return; // couldn't solve, oh dear, never mind
    }
    
    // Now feels like a good time to check solution wrt target doesn't depend on target
    ASSERT( val->IsIndependentOf( target ) );        
    
    // Reduce out any "not" in the key expression, updating the sense
    while( auto nk = dynamic_pointer_cast<NotOperator>( key ) )
    {
        key = OnlyElementOf( nk->GetBooleanOperands() );
        key_sense = !key_sense;
    }
    
//    FTRACEC("Yes, solved value: ")(val)("\n");
    
    // Add it
    psol[key_sense][key] = val;
}                                            


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


shared_ptr<Expression> AndOperator::TrySolveFor( shared_ptr<Expression> target ) const
{
    // With AndOperator, solving via any clause solves the whole thing. So try that first.
    for( shared_ptr<BooleanExpression> op : sa )
        if( shared_ptr<Expression> solution = op->TrySolveFor( target ) )
            return solution;
            
    // Didn't work so fall back to BooleanOperator's solver which will try to do it using 
    // partial solutions.
    // Note: intercept pattern: with other boolean operators, BooleanOperator::TrySolveFor()
    // is not overridden and therefore called directly. It tries to generate a solution
    // from that operator's partial solution. Here, though, we intercept in case we could
    // get a solution from a clause, and only resort to partials if that fails.
    return BooleanOperator::TrySolveFor( target );
}


BooleanExpression::PartialSolution AndOperator::PartialSolveFor( shared_ptr<Expression> target ) const
{
    PartialSolution and_psol;
    
    for( shared_ptr<BooleanExpression> a : sa )
    {
        // Simply merge partial solutions together
        PartialSolution a_psol = a->PartialSolveFor(target);
        and_psol[true] = UnionOf( and_psol[true], a_psol[true] );
        and_psol[false] = UnionOf( and_psol[false], a_psol[false] );
    }

    return and_psol;
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


BooleanExpression::PartialSolution OrOperator::PartialSolveFor( shared_ptr<Expression> target ) const
{
    PartialSolution psol;
    
    ForAllCommutativeDistinctPairs( sa, [&](shared_ptr<BooleanExpression> a,
                                            shared_ptr<BooleanExpression> b) 
    {
        TryAddPartialSolutionFor( target, psol, false, a, b );
        TryAddPartialSolutionFor( target, psol, false, b, a );
    } );

    return psol;
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


shared_ptr<Expression> BoolEqualOperator::TrySolveFor( shared_ptr<Expression> target ) const
{
    shared_ptr<BooleanExpression> indep_op;
    bool found_me = false;
    for( auto op : list<shared_ptr<BooleanExpression>>{a, b} )
    {
        bool indep = op->IsIndependentOf( target );
        bool is_me = (OrderCompare( op, target ) == EQUAL);        
        ASSERT( !(indep && is_me) ); 
        if( indep )
            indep_op = op;
        if( is_me )
            found_me = true;                  
    }
    
    if( found_me )
        return indep_op;
    else
        return nullptr;
}


BooleanExpression::PartialSolution BoolEqualOperator::PartialSolveFor( shared_ptr<Expression> target ) const
{
    PartialSolution psol;
    
    TryAddPartialSolutionFor( target, psol, true, a, b );
    TryAddPartialSolutionFor( target, psol, true, b, a );

    return psol;
}


string BoolEqualOperator::Render() const
{
    return RenderForMe(a) + " iff " + RenderForMe(b);
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


BooleanExpression::PartialSolution ImplicationOperator::PartialSolveFor( shared_ptr<Expression> target ) const
{
    PartialSolution psol;
    
    TryAddPartialSolutionFor( target, psol, true, a, b );
    //TryAddPartialSolutionFor( target, psol, true, b, a ); // would need to negate a, which we can't then solve; TBD

    return psol;
}


string ImplicationOperator::Render() const
{
    return RenderForMe(a)+" ⇒ "+RenderForMe(b);
}


Expression::Precedence ImplicationOperator::GetPrecedence() const
{
    return Precedence::IMPLICATION;
}

// ------------------------- BooleanConditionalOperator --------------------------

BooleanConditionalOperator::BooleanConditionalOperator( shared_ptr<BooleanExpression> a_,
                                                        shared_ptr<BooleanExpression> b_,
                                                        shared_ptr<BooleanExpression> c_ ) :
    a( a_ ),
    b( b_ ),
    c( c_ )
{   
}    


list<shared_ptr<BooleanExpression>> BooleanConditionalOperator::GetBooleanOperands() const
{
    return { a, b, c };
}


shared_ptr<BooleanResult> BooleanConditionalOperator::Evaluate( const EvalKit &kit ) const
{
    shared_ptr<BooleanResult> ra = a->Evaluate(kit);   
    switch( ra->value )
    {   
    case BooleanResult::FALSE:
        return c->Evaluate(kit);
    case BooleanResult::UNDEFINED:
        {
            shared_ptr<BooleanResult> rb = b->Evaluate(kit);   
            shared_ptr<BooleanResult> rc = c->Evaluate(kit);   
            if( rb->value == rc->value )
                return rb; // not ambiguous if both options are the same
            return make_shared<BooleanResult>( BooleanResult::UNDEFINED );
        }
    case BooleanResult::TRUE:
        return b->Evaluate(kit);
    default:
        ASSERTFAIL("Missing case")
    }  
}


BooleanExpression::PartialSolution BooleanConditionalOperator::PartialSolveFor( shared_ptr<Expression> target ) const
{
    PartialSolution psol;
    
    TryAddPartialSolutionFor( target, psol, true, a, b );
    TryAddPartialSolutionFor( target, psol, false, a, c ); 

    return psol;
}


string BooleanConditionalOperator::Render() const
{
    return RenderForMe(a) + " ? " + RenderForMe(b) + " : " + RenderForMe(c);
}


Expression::Precedence BooleanConditionalOperator::GetPrecedence() const
{
    return Precedence::CONDITIONAL;
}
