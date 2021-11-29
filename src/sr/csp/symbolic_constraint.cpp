#include "symbolic_constraint.hpp"

#include "query.hpp"
#include "agents/agent.hpp"
#include "link.hpp"

using namespace CSP;


SymbolicConstraint::Plan::Plan( SymbolicConstraint *algo_,
                                shared_ptr<SYM::BooleanOperator> op_,
                                set<SR::PatternLink> relevent_plinks ) :
    algo( algo_ ),
    op( op_ )
{
    DetermineVariables( relevent_plinks );       
}


SymbolicConstraint::SymbolicConstraint( shared_ptr<SYM::BooleanOperator> op,
                                        set<SR::PatternLink> relevent_plinks ) :
    plan( this, op, relevent_plinks )
{
}


void SymbolicConstraint::Plan::DetermineVariables( set<SR::PatternLink> relevent_plinks )
{ 
    // The keyer
    set<SR::PatternLink> input_plinks = op->GetInputPatternLinks();
    set<SR::PatternLink> my_input_plinks = IntersectionOf( input_plinks, relevent_plinks );
    for( VariableId plink : my_input_plinks )
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


void SymbolicConstraint::Test( Assignments frees_map )
{   
    INDENT("T");

    // Merge incoming values with the forces to get a full set of 
    // values that must tally up with the links required by the operator.
    SR::SolutionMap full_map;
    full_map = UnionOfSolo(forces_map, frees_map);

    {
        //Tracer::RAIIDisable silencer(); // make queries be quiet

        SYM::Operator::EvalKit kit { &full_map, knowledge };
        ASSERT(plan.op);
        plan.op->Evaluate( kit );
    }            
}


void SymbolicConstraint::Dump() const
{
    TRACE("Degree %d\n", 
          plan.variables.size());
    TRACEC("Expression: ")(plan.op->Render())("\n");
    TRACEC("Variables: ")(plan.variables)("\n");
}      
