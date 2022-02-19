#include "conditional_operators.hpp"
#include "boolean_operators.hpp"
#include "primary_expressions.hpp"
#include "result.hpp"

using namespace SYM;

// ------------------------- ConditionalOperator --------------------------

ConditionalOperator::ConditionalOperator( shared_ptr<BooleanExpression> control_,
                                          shared_ptr<SymbolExpression> option_true_,
                                          shared_ptr<SymbolExpression> option_false_ ) :
    control( control_ ),
    option_true( option_true_ ),
    option_false( option_false_ )
{
}

    
list<shared_ptr<Expression>> ConditionalOperator::GetOperands() const
{
    return {control, option_true, option_false};
}


shared_ptr<SymbolResultInterface> ConditionalOperator::Evaluate( const EvalKit &kit ) const
{
    shared_ptr<BooleanResultInterface> r_control = control->Evaluate(kit);   
    if( r_control->IsDefinedAndUnique() )
    {
        if( r_control->GetAsBool() ) // TRUE
        {
            return option_true->Evaluate(kit);
        }
        else // FALSE
        {
            return option_false->Evaluate(kit);
        }
    }
    else // UNDEFINED
    {
        shared_ptr<SymbolResultInterface> r_option_true = option_true->Evaluate(kit);   
        shared_ptr<SymbolResultInterface> r_option_false = option_false->Evaluate(kit);   
        if( *r_option_true == *r_option_false )
            return r_option_true; // not ambiguous if both options are the same
        return make_shared<SymbolResult>( ResultInterface::UNDEFINED );
    }
}


string ConditionalOperator::Render() const
{
    return RenderForMe(control) + " ? " + RenderForMe(option_true) + " : " + RenderForMe(option_false);
}


Expression::Precedence ConditionalOperator::GetPrecedence() const
{
    return Precedence::CONDITIONAL;
}

// ------------------------- MultiConditionalOperator --------------------------

MultiConditionalOperator::MultiConditionalOperator( vector<shared_ptr<BooleanExpression>> controls_,
                                                    vector<shared_ptr<SymbolExpression>> options_ ) :
    controls( controls_ ),
    options( options_ )
{
    ASSERT( options.size() == 1<<controls.size() );
}

    
list<shared_ptr<Expression>> MultiConditionalOperator::GetOperands() const
{
    list<shared_ptr<Expression>> ops;
    for( shared_ptr<BooleanExpression> c : controls )
        ops.push_back( c );
    for( shared_ptr<SymbolExpression> o : options )
        ops.push_back( o );

    return ops;
}


shared_ptr<SymbolResultInterface> MultiConditionalOperator::Evaluate( const EvalKit &kit ) const
{
    unsigned int int_control = 0;
    for( int i=0; i<controls.size(); i++ )
    {
        shared_ptr<BooleanResultInterface> r = controls[i]->Evaluate(kit);
        
        // Abort if any controls evaluate undefined (TODO could do better)
        if( !r->IsDefinedAndUnique() )
            return make_shared<SymbolResult>( ResultInterface::UNDEFINED );
            
        int_control |= (int)r->GetAsBool() << i;
    }
    
    return options[int_control]->Evaluate(kit);
}


string MultiConditionalOperator::Render() const
{
    list<string> str_controls;
    for( shared_ptr<BooleanExpression> c : controls )
        str_controls.push_back( c->Render() );
        
    list<string> str_options;
    for( shared_ptr<SymbolExpression> o : options )
        str_options.push_back( o->Render() );
        
    return Join(str_controls, ", ", "[", "]") + 
           "?:" + 
           Join(str_options, ", ", "[", "]");
}


Expression::Precedence MultiConditionalOperator::GetPrecedence() const
{
    return Precedence::CONDITIONAL;
}



