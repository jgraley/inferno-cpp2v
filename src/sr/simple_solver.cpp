#include "simple_solver.hpp"
#include "query.hpp"
#include "agent.hpp"

#include <algorithm>

using namespace CSP;


SimpleSolver::SimpleSolver( const list< shared_ptr<Constraint> > &constraints_ ) :
    constraints(constraints_),
    variables(DeduceVariables(constraints))
{
}


void SimpleSolver::Start( const list< TreePtr<Node> > &initial_domain_ )
{
    initial_domain = initial_domain_;

    assignments.clear();
    for( VariableId v : variables )
    {
        assignments[v] = NullValue; // means "unassigned"
    }
    matches.clear();
    
    TryVariable( variables.begin() );    
}


bool SimpleSolver::GetNextSolution( map< shared_ptr<Constraint>, list< Value > > *values, 
                                    SideInfo *side_info )
{
    if( matches.empty() )
        return false;
    if( values )
    {
        values->clear();
        for( shared_ptr<Constraint> c : constraints )
        {
            (*values)[c] = GetConstraintValues(c, matches.front().first);
            ASSERT( values->at(c).size() == c->GetDegree() ); // Assignments should be complete
        }
    }
    if( side_info )
        *side_info = *matches.front().second;
    matches.pop_front();
    return true;
}


list<VariableId> SimpleSolver::DeduceVariables( const list< shared_ptr<Constraint> > &constraints )
{
    list<VariableId> variables;
    for( shared_ptr<Constraint> c : constraints )
    {
        list<VariableId> vars = c->GetVariables();
        
        for( VariableId v : vars )
        {
            if( find(variables.begin(), variables.end(), v) == variables.end() )
                variables.push_back( v );
        }
    }
    return variables;
}


bool SimpleSolver::TryVariable( list<VariableId>::const_iterator current )
{
    list<VariableId>::const_iterator next = current;
    ++next;
    bool complete = (next == variables.end());
    
    for( Value v : initial_domain )
    {
        assignments[*current] = v;
     
        shared_ptr<SideInfo> side_info;
        if( complete )
            side_info = make_shared<SideInfo>();        
        
        bool ok = Test( assignments, side_info.get() ); // Ask for side info if assignment is complete
        
        if( !ok )
            continue; // backtrack
            
        if( complete )
            matches.push_back( make_pair(assignments, side_info) );
        else
            TryVariable( next );

    }
    return false;

}

bool SimpleSolver::Test( map<VariableId, Value> &assigns, 
                         SideInfo *side_info )
{
    bool ok = true; // AND-rule
    for( shared_ptr<Constraint> c : constraints )
    {
        list<Value> vals = GetConstraintValues( c, assignments );

        if( vals.size() < c->GetDegree() )
            continue; // There were unassigned variables, don't bother testing this constraint
            
        ok = c->Test(vals, side_info); 
        
        if( !ok )
            break;
    }    
    return ok;
}


list<Value> SimpleSolver::GetConstraintValues( shared_ptr<Constraint> c, const Assignments &a )
{
    list<VariableId> vars = c->GetVariables();
    list<Value> vals;
    for( VariableId var : vars )
    {
        Value val = a.at(var); 
        if( val == NullValue )
            continue; // unassigned variable
        vals.push_back( val );
    }    
    return vals;
}
