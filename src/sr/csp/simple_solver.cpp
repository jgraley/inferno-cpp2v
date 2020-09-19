#include "simple_solver.hpp"
#include "solver_holder.hpp"
#include "query.hpp"
#include "agents/agent.hpp"

#include <algorithm>

#define TRACK_BEST_ASSIGNMENT

using namespace CSP;


SimpleSolver::Plan::Plan( const list< shared_ptr<Constraint> > &constraints_, 
                          const list<VariableId> *variables_ ) :
    constraints(constraints_)
{
    DeduceVariables(variables_);
}


void SimpleSolver::Plan::DeduceVariables( const list<VariableId> *variables_ )
{
    list<VariableId> my_variables;
    set<VariableId> variables_check_set;

    for( shared_ptr<Constraint> c : constraints )
    {
        list<VariableId> vars = c->GetFreeVariables();
        
        for( VariableId v : vars )
        {
            if( variables_check_set.count(v) == 0 )
            {
                variables_check_set.insert( v );
                if( variables_ == nullptr )
                    my_variables.push_back( v ); 
            }
        }
    }
    
    if( variables_ == nullptr )
    {
        // No variables list was supplied, so return the one we generated
        variables = my_variables;
    }
    else
    {
        // A variables list was supplied and it must have the same set of variables
        for( VariableId v : *variables_ )
            ASSERT( variables_check_set.count(v) == 1 );
        variables = *variables_;
    }
}


SimpleSolver::SimpleSolver( const list< shared_ptr<Constraint> > &constraints_, 
                            const list<VariableId> *variables_ ) :
    plan( constraints_, variables_ ),
    holder(nullptr)
{
    TraceProblem();
}
                        

void SimpleSolver::Run( ReportageObserver *holder_, const set< TreePtr<Node> > &initial_domain_ )
{
    ASSERT(holder==nullptr)("You can bind a solver to more than one holder, but you obviously can't overlap their Run()s, stupid.");
    holder = holder_;
    initial_domain = initial_domain_;

    assignments.clear();
    
#ifdef TRACK_BEST_ASSIGNMENT    
    best_assignments.clear();
    best_num_assignments = 0;
#endif    
    
    TryVariable( plan.variables.begin() );    

#ifdef TRACK_BEST_ASSIGNMENT    
    ShowBestAssignment();
#endif

    holder = nullptr;
}


bool SimpleSolver::TryVariable( list<VariableId>::const_iterator current )
{
    list<VariableId>::const_iterator next = current;
    ++next;
    bool complete = (next == plan.variables.end());
    
    for( Value v : initial_domain )
    {
        assignments[*current] = v;
     
        bool ok = Test( assignments, *current );
        
        if( !ok )
        {
            assignments.erase(*current); // remove failed variable 
            continue; // backtrack
        }
            
#ifdef TRACK_BEST_ASSIGNMENT    
        int num=0;
        for( auto p : assignments )
            num++;
        if( num > best_num_assignments )
        {
             best_assignments = assignments;
             best_num_assignments = num;
        }
#endif
        
        if( complete )
            ReportSolution( assignments );
        else
            TryVariable( next );
    }
    return false;
}


bool SimpleSolver::Test( const Assignments &assigns, 
                         VariableId variable_of_interest )
{
    bool ok = true; // AND-rule
    for( shared_ptr<Constraint> c : plan.constraints )
    {
        list<Value> vals = GetValuesForConstraint( c, assignments );

        if( vals.size() < c->GetFreeDegree() )
            continue; // There were unassigned variables, don't bother testing this constraint
            
        TimedOperations(); // overhead should be hidden by Constraint::Test()
        ok = c->Test(vals); 
        
        if( !ok )
            break;
                            
        // Only expected to pass for base (which we only know about if 
        // it's FREE). We don't check whether the other 
        // values accepted by the constraint are compatible with the 
        // variable's deduced type, because that is the job of the 
        // constraint that is based upon it.
        // for( VariableId var : c->GetFreeVariables() )
        //     CheckLocalMatch( assigns, var );   
    } 

    return ok;
}


list<Value> SimpleSolver::GetValuesForConstraint( shared_ptr<Constraint> c, const Assignments &a )
{
    list<VariableId> vars = c->GetFreeVariables();
    list<Value> vals;
    for( VariableId var : vars )
    {
        if( a.count(var) > 0 )
        {
            Value val = a.at(var); 
            vals.push_back( val );
        }
    }    
    return vals;
}


void SimpleSolver::ReportSolution( const Assignments &assignments )
{
    map< shared_ptr<Constraint>, list< Value > > vals;
    vals.clear();
    for( shared_ptr<Constraint> c : plan.constraints )
    {
        vals[c] = GetValuesForConstraint(c, assignments);
        ASSERT( vals.at(c).size() == c->GetFreeDegree() ); // Assignments should be complete
    }
    
    holder->ReportSolution( vals );
}                     


void SimpleSolver::TraceProblem() const
{
    TRACEC("SimpleSolver; %d constraints", plan.constraints.size());
    INDENT(" ");
    for( shared_ptr<Constraint> c : plan.constraints )
    {
        c->TraceProblem();
    }
}


void SimpleSolver::CheckLocalMatch( const Assignments &assignments, VariableId variable )
{
    ASSERT( assignments.count(variable) > 0 );   // in the assignment
    ::CheckLocalMatch( variable, assignments.at(variable) );
}


void SimpleSolver::ShowBestAssignment()
{
    if( best_assignments.empty() )
        return; // didn't get around to updating it yet
    TRACE("Best assignment assigned %d of %d variables:\n", best_assignments.size(), plan.variables.size());
    INDENT(" ");
    for( VariableId var : plan.variables )
    {
        if( best_assignments.count(var) > 0 )
        {
            TRACEC("Variable ")(*var)(" assigned ")(*best_assignments.at(var));
            if( var->IsLocalMatch(best_assignments.at(var).get()) )
            {
                TRACEC(" is a local match\n");
            }
            else
            {
                TRACEC(" is not a local match (two reasons this might be OK)\n");            
                ASSERT(best_assignments.size() < plan.variables.size())("local mismatch in passing complete assignment");
            }
            // Reason 1: At the point we gave up, no constraint containing this 
            // variable was complete (ie had a full set of assigned variables)
            // Reason 2: Complete constraints contained this variable but that
            // didn't include one which had the variable as its base, and so 
            // local match was not enforced.
        }
        else 
        {
            TRACEC("Variable ")(*var)(" could not be assigned a consistent value.\n");
            break; // later ones won't even have been tried
        }
    }
}


void SimpleSolver::TimedOperations()
{
    auto now = chrono::steady_clock::now();
    auto since = now - last_report;
    if( since > chrono::seconds(3) )
    {
        ShowBestAssignment();
        last_report = now;
    }
}

