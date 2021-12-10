#include "symbolic_constraint.hpp"

#include "query.hpp"
#include "agents/agent.hpp"
#include "link.hpp"
#include "../sym/comparison_operators.hpp"
#include "../sym/primary_expressions.hpp"

using namespace CSP;


SymbolicConstraint::SymbolicConstraint( shared_ptr<SYM::BooleanExpression> op,
                                        set<SR::PatternLink> relevent_plinks ) :
    plan( this, op, relevent_plinks )
{
}


SymbolicConstraint::Plan::Plan( SymbolicConstraint *algo_,
                                shared_ptr<SYM::BooleanExpression> op_,
                                set<SR::PatternLink> relevent_plinks ) :
    algo( algo_ ),
    op( op_ )
{
    DetermineVariables( relevent_plinks );       
}


void SymbolicConstraint::Plan::DetermineVariables( set<SR::PatternLink> relevent_plinks )
{ 
    // The keyer
    set<SR::PatternLink> required_plinks = op->GetRequiredPatternLinks();
    
    // Filter down to variables relevent to the current solver
    set<SR::PatternLink> my_required_plinks = IntersectionOf( required_plinks, relevent_plinks );
    for( VariableId plink : my_required_plinks )
        variables.push_back( plink );     
}


string SymbolicConstraint::Plan::GetTrace() const 
{
    return algo->GetName() + ".plan";
}


const list<VariableId> &SymbolicConstraint::GetVariables() const
{ 
    return plan.variables;
}


void SymbolicConstraint::Start( const Assignments &forces_map_, 
                                const SR::TheKnowledge *knowledge_ )
{
    forces_map = forces_map_;
    knowledge = knowledge_;
    ASSERT( knowledge );
}   


tuple<bool, Assignment> SymbolicConstraint::Test( Assignments frees_map,
                                                  VariableId current_var )
{   
    INDENT("T");

    // Merge incoming values with the forces to get a full set of 
    // values that must tally up with the links required by the operator.
    SR::SolutionMap full_map;
    full_map = UnionOfSolo(forces_map, frees_map);
    
    //Tracer::RAIIDisable silencer(); // make queries be quiet

    SYM::Expression::EvalKit kit { &full_map, knowledge };
    ASSERT(plan.op);
    SYM::BooleanResult r = plan.op->Evaluate( kit );
    if( r.matched == SYM::BooleanResult::TRUE || r.matched == SYM::BooleanResult::UNKNOWN )
    {
        return make_tuple(true, Assignment());
    }
    else
    {
        if( current_var )
        {
            if( auto eq = dynamic_pointer_cast<SYM::EqualsOperator>(plan.op) )
            {
                shared_ptr<SYM::SymbolExpression> other;
                for( shared_ptr<SYM::Expression> op : eq->GetOperands() )
                {
                    bool is_curr = false;
                    if( auto sv_op = dynamic_pointer_cast<SYM::SymbolVariable>(op) )
                        if( OnlyElementOf( sv_op->GetRequiredPatternLinks() ) == current_var )
                            is_curr = true;
                    if( !is_curr )
                        other = dynamic_pointer_cast<SYM::SymbolExpression>(op);                    
                }
                ASSERT( other )
                      ("didn't find any other operands or not a symbol expression, current_var=")(current_var)
                      ("expression:\n")(plan.op); 

                SYM::SymbolResult other_result = other->Evaluate( kit );
                if( other_result.xlink ) // Evaluate was ambiguous due missing assignments?
                    return make_tuple(false, SR::LocatedLink( current_var, other_result.xlink ));            
            }
        }
        
        try
        {
            if( r.reason )
                rethrow_exception(r.reason);            
        }  
        catch( const ::Mismatch &e )
        {
#ifdef HINTS_IN_EXCEPTIONS   
            if( auto pae = dynamic_cast<const SR::Agent::Mismatch *>(&e) ) // could have a hint            
                return make_tuple( false, pae->hint );
#endif            
        }
        return make_tuple(false, Assignment());
    }
}


void SymbolicConstraint::Dump() const
{
    TRACE("Degree %d\n", 
          plan.variables.size());
    TRACEC("Expression: ")(plan.op->Render())("\n");
    TRACEC("Variables: ")(plan.variables)("\n");
}      
