#include "simple_solver.hpp"
#include "query.hpp"
#include "agent.hpp"

#include <algorithm>

using namespace SR;


SimpleSolver::SimpleSolver( const list<Constraint *> &constraints_, 
                            const list< TreePtr<Node> > &initial_domain_ ) :
    constraints(constraints_),
    initial_domain(initial_domain_),
    variables(DeduceVariables(constraints))
{
}


void SimpleSolver::Start()
{
    assignments.clear();
    for( Constraint::VariableId v : variables )
    {
        assignments[v] = Constraint::NullValue; // means "unassigned"
    }
    current = variables.begin();
}


bool SimpleSolver::GetNextSolution( std::list< TreePtr<Node> > *values, 
                                    Constraint::SideInfo *side_info )
{
    for( Constraint::Value v : initial_domain )
    {
        assignments[*current] = v;
        
        bool ok = Test(assignments); // Ask for side data if assignment is complete
        
        // Three possibilities
        // - Fail: backtrack (continue iterating; at end return to caller with previous variable, at initial frame fail out)
        // - Hit but incomplete assignment: Recurse into next variable
        // - Hit and complete: AWESOME!!!, we've MADE IT!!! but can continue to look for more hits: co-routine suspend or state-out?
    }
    return false;
}


list<Constraint::VariableId> SimpleSolver::DeduceVariables( const list<Constraint *> &constraints )
{
    list<Constraint::VariableId> variables;
    for( Constraint *c : constraints )
    {
        list<Constraint::VariableId> vars = c->GetVariables();
        
        for( Constraint::VariableId v : vars )
        {
            if( std::find(variables.begin(), variables.end(), v) == variables.end() )
                variables.push_back( v );
        }
    }
    return variables;
}


bool SimpleSolver::Test( std::map<Constraint::VariableId, Constraint::Value> &assigns ) // TODO side_data: merge it from all the Tests
{
    bool ok = true; // AND-rule
    for( Constraint *c : constraints )
    {
        list<Constraint::VariableId> vars = c->GetVariables();
        list<Constraint::Value> vals;
        for( Constraint::VariableId var : vars )
        {
            Constraint::Value val = assignments.at(var); 
            if( val == Constraint::NullValue )
                break; // unassigned variable
            vals.push_back( val );
        }
        
        if( vals.size() < c->GetDegree() )
            continue; // There were unassigned variables, don't bother testing this constraint
            
        ok = c->Test(vals); 
        
        if( !ok )
            break;
    }    
    return ok;
}
