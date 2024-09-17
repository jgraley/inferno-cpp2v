#include "conditional_operators.hpp"
#include "boolean_operators.hpp"
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
    if( option_false )
        return {control, option_true, option_false};
    else
        return {control, option_true};
}


unique_ptr<SymbolicResult> ConditionalOperator::Evaluate( const EvalKit &kit ) const
{
    unique_ptr<BooleanResult> r_control = control->Evaluate(kit);   
    if( r_control->IsDefinedAndUnique() )
    {
        if( r_control->GetAsBool() ) // TRUE
        {
            return option_true->Evaluate(kit);
        }
        else // FALSE
        {
            if( option_false )
                return option_false->Evaluate(kit);
            else
                return make_unique<EmptyResult>();
        }
    }
    else // UNDEFINED
    {
        unique_ptr<SymbolicResult> r_option_true = option_true->Evaluate(kit);   
        unique_ptr<SymbolicResult> r_option_false = option_false->Evaluate(kit);   
        if( *r_option_true == *r_option_false )
            return r_option_true; // not ambiguous if both options are the same
        return make_unique<EmptyResult>();
    }
}


string ConditionalOperator::Render() const
{
    if( option_false )
        return RenderForMe(control) + " → ⎨" + RenderForMe(option_false) + ", " + RenderForMe(option_true) + "⎬";
    else
        return RenderForMe(control) + " → " + RenderForMe(option_true);    
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


unique_ptr<SymbolicResult> MultiConditionalOperator::Evaluate( const EvalKit &kit ) const
{
    unsigned int int_control = 0;
    for( int i=0; i<controls.size(); i++ )
    {
        unique_ptr<BooleanResult> r = controls[i]->Evaluate(kit);
        
        // Abort if any controls evaluate undefined (TODO could do better)
        if( !r->IsDefinedAndUnique() )
            return make_unique<EmptyResult>();
            
        int_control |= (int)r->GetAsBool() << i;
    }
        
    unique_ptr<SymbolicResult> result = options[int_control]->Evaluate(kit);
    //FTRACE("Option %d is:", int_control)(options[int_control])("\n")
    //      ("Evaluates to:")(result)("\n");
    return result;
}


string MultiConditionalOperator::Render() const
{
    list<string> str_controls;
    for( shared_ptr<BooleanExpression> c : controls )
        str_controls.push_back( c->Render() );
        
    list<string> str_options;
    for( shared_ptr<SymbolExpression> o : options )
        str_options.push_back( o->Render() );
        
    return Join(str_controls, ", ", "⎨", "⎬") + 
           " → " + 
           Join(str_options, ", ", "⎨", "⎬");
}


Expression::Precedence MultiConditionalOperator::GetPrecedence() const
{
    return Precedence::CONDITIONAL;
}



