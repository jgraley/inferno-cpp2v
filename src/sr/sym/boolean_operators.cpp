#include "boolean_operators.hpp"
#include "predicate_operators.hpp"
#include "conditional_operators.hpp"
#include "symbol_operators.hpp"
#include "rewriters.hpp"
#include "result.hpp"
#include <algorithm>

using namespace SYM;

#define SOLVE_FROM_PARTIALS_CHECKING

// ------------------------- BooleanConstant --------------------------

BooleanConstant::BooleanConstant( bool value_ ) :
    value( value_ )
{
}


unique_ptr<BooleanResult> BooleanConstant::Evaluate( const EvalKit &kit ) const
{
    return make_unique<BooleanResult>( value );
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

// ------------------------- NotOperator --------------------------

NotOperator::NotOperator( shared_ptr<BooleanExpression> a_ ) :
    a( a_ )
{   
}    


list<shared_ptr<BooleanExpression>> NotOperator::GetBooleanOperands() const
{
    return { a };
}


unique_ptr<BooleanResult> NotOperator::Evaluate( const EvalKit &kit,
                                                 list<unique_ptr<BooleanResult>> &&op_results ) const
{
    unique_ptr<BooleanResult> ra = move( op_results.front() );
    if( ra->IsDefinedAndUnique() ) // DEFINED
    {
        return make_unique<BooleanResult>( !ra->GetAsBool() );
    }
    else // UNDEFINED
    {
        return ra; // UNDEFINED again
    } 
}


string NotOperator::Render() const
{
    return "¬"+RenderForMe(a);
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


unique_ptr<BooleanResult> AndOperator::Evaluate( const EvalKit &kit,
                                                 list<unique_ptr<BooleanResult>> &&op_results ) const
{
    if( op_results.empty() )
        return make_unique<BooleanResult>( true ); // identity element
        
    // Lower certainly dominates
    return move( *min_element( op_results.begin(), 
                               op_results.end(), 
                               DereferencingCompare<unique_ptr<BooleanResult>> ) );
}


bool AndOperator::IsCommutative() const
{
    return true;
}


string AndOperator::Render() const
{
    if( sa.empty() )
        return "TRUE(empty-And)";
    list<string> ls;
    for( shared_ptr<BooleanExpression> a : sa )
        ls.push_back( RenderForMe(a) );
    return Join( ls, " ∧ " );
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


unique_ptr<BooleanResult> OrOperator::Evaluate( const EvalKit &kit,
                                                         list<unique_ptr<BooleanResult>> &&op_results ) const
{
    if( op_results.empty() )
        return make_unique<BooleanResult>( false ); // identity element

    // Higher certainly dominates
    return move( *max_element( op_results.begin(), 
                               op_results.end(), 
                               DereferencingCompare<unique_ptr<BooleanResult>> ) );
}


bool OrOperator::IsCommutative() const
{
    return true;
}


string OrOperator::Render() const
{
    if( sa.empty() )
        return "FALSE(empty-Or)";
    list<string> ls;
    for( shared_ptr<BooleanExpression> a : sa )
        ls.push_back( RenderForMe(a) );
    return Join( ls, " ∨ " );
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


unique_ptr<BooleanResult> BoolEqualOperator::Evaluate( const EvalKit &kit,
                                                       list<unique_ptr<BooleanResult>> &&op_results ) const
{
    unique_ptr<BooleanResult> ra = move( op_results.front() );
    unique_ptr<BooleanResult> rb = move( op_results.back() );
    
    if( !ra->IsDefinedAndUnique() )
        return ra;
        
    if( !rb->IsDefinedAndUnique() )
        return rb;
    
    bool res = ( ra->GetAsBool() == rb->GetAsBool() );
    return make_unique<BooleanResult>( res );     
}


bool BoolEqualOperator::IsCommutative() const
{
    return true;
}

string BoolEqualOperator::Render() const
{
    return RenderForMe(a) + " ⇔ " + RenderForMe(b);
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


unique_ptr<BooleanResult> ImplicationOperator::Evaluate( const EvalKit &kit,
                                                         list<unique_ptr<BooleanResult>> &&op_results ) const
{
    unique_ptr<BooleanResult> ra = move( op_results.front() );
    unique_ptr<BooleanResult> rb = move( op_results.back() );
    if( ra->IsDefinedAndUnique() )
    {
        if( ra->GetAsBool() ) // TRUE
        {
            return rb;
        }
        else // FALSE
        {
            return make_unique<BooleanResult>( true );
        }
    }
    else // UNDEFINED
    {
        if( rb->IsDefinedAndTrue() )
            return rb; // TRUE again
        else
            return ra; // UNDEFINED again
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

// ------------------------- Uniplexor --------------------------

Uniplexor::Uniplexor( shared_ptr<BooleanExpression> a_,
                      shared_ptr<BooleanExpression> b_,
                      shared_ptr<BooleanExpression> c_ ) :
    a( a_ ),
    b( b_ ),
    c( c_ )
{   
}    


list<shared_ptr<BooleanExpression>> Uniplexor::GetBooleanOperands() const
{
    return { a, b, c };
}


unique_ptr<BooleanResult> Uniplexor::Evaluate( const EvalKit &kit ) const
{
    unique_ptr<BooleanResult> ra = a->Evaluate(kit);   
    if( ra->IsDefinedAndUnique() )
    {
        if( ra->GetAsBool() ) // TRUE
        {
            return b->Evaluate(kit);
        }
        else // FALSE
        {
            return c->Evaluate(kit);
        }
    }
    else // UNDEFINED
    {
        unique_ptr<BooleanResult> rb = b->Evaluate(kit);   
        unique_ptr<BooleanResult> rc = c->Evaluate(kit);   
        if( *rb == *rc )
            return rb; // not ambiguous if both options are the same
        return make_unique<BooleanResult>( false );
    }
}


string Uniplexor::Render() const
{
    return RenderForMe(a) + " → ⎨" + RenderForMe(c) + ", " + RenderForMe(b) + "⎬";
}


Expression::Precedence Uniplexor::GetPrecedence() const
{
    return Precedence::CONDITIONAL;
}

// ------------------------- Multiplexor --------------------------

Multiplexor::Multiplexor( vector<shared_ptr<BooleanExpression>> controls_,
                          vector<shared_ptr<BooleanExpression>> options_ ) :
    controls( controls_ ),
    options( options_ )
{
    ASSERT( options.size() == 1<<controls.size() );
}

    
list<shared_ptr<BooleanExpression>> Multiplexor::GetBooleanOperands() const
{
    list<shared_ptr<BooleanExpression>> ops;
    for( shared_ptr<BooleanExpression> c : controls )
        ops.push_back( c );
    for( shared_ptr<BooleanExpression> o : options )
        ops.push_back( o );

    return ops;
}


unique_ptr<BooleanResult> Multiplexor::Evaluate( const EvalKit &kit ) const
{
    unsigned int int_control = 0;
    for( int i=0; i<controls.size(); i++ )
    {
        unique_ptr<BooleanResult> r = controls[i]->Evaluate(kit);
        
        // Abort if any controls evaluate undefined (TODO could do better)
        if( !r->IsDefinedAndUnique() )
            return make_unique<BooleanResult>( false );
            
        int_control |= (int)r->GetAsBool() << i;
    }
    
    return options[int_control]->Evaluate(kit);
}


string Multiplexor::Render() const
{
    list<string> str_controls;
    for( shared_ptr<BooleanExpression> c : controls )
        str_controls.push_back( c->Render() );
        
    list<string> str_options;
    for( shared_ptr<BooleanExpression> o : options )
        str_options.push_back( o->Render() );
        
    return Join(str_controls, ", ", "⎨", "⎬") + 
           " → " + 
           Join(str_options, ", ", "⎨", "⎬");
}


Expression::Precedence Multiplexor::GetPrecedence() const
{
    return Precedence::CONDITIONAL;
}


