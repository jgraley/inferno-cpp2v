#include "simple_solver.hpp"
#include "solver_holder.hpp"
#include "query.hpp"
#include "agent.hpp"

#include <algorithm>

#define TRACK_BEST_ASSIGNMENT

using namespace CSP;

SimpleSolver::SimpleSolver( const list< shared_ptr<Constraint> > &constraints_, 
                            const list<VariableId> *variables_ ) :
    holder(nullptr),
    constraints(constraints_),
    variables( DeduceVariables(constraints, variables_) )
{
    TraceProblem();
}


list<VariableId> SimpleSolver::DeduceVariables( const list< shared_ptr<Constraint> > &constraints, 
                                                const list<VariableId> *variables_ )
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
        return my_variables;
    }
    else
    {
        // A variables list was supplied and it must have the same set of variables
        for( VariableId v : *variables_ )
            ASSERT( variables_check_set.count(v) == 1 );
        return *variables_;
    }
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
    
    TryVariable( variables.begin() );    

#ifdef TRACK_BEST_ASSIGNMENT    
    ShowBestAssignment();
#endif

    holder = nullptr;
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
        
        bool ok = Test( assignments, *current, side_info.get() ); // Ask for side info if assignment is complete
        
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
            ReportSolution( assignments, side_info );
        else
            TryVariable( next );
    }
    return false;
}


bool SimpleSolver::Test( const Assignments &assigns, 
                         VariableId variable_of_interest,
                         SideInfo *side_info )
{
    bool ok = true; // AND-rule
    for( shared_ptr<Constraint> c : constraints )
    {
        list<Value> vals = GetValuesForConstraint( c, assignments );

        if( vals.size() < c->GetFreeDegree() )
            continue; // There were unassigned variables, don't bother testing this constraint
            
        TimedOperations(); // overhead should be hidden by Constraint::Test()
        ok = c->Test(vals, side_info); 
        
        if( !ok )
            break;
                            
        // Only expected to pass for base (which we only know about if 
        // it's FREE). We don't check whether the other 
        // values accepted by the constraint are compatible with the 
        // variable's deduced type, because that is the job of the 
        // constraint that is based upon it.
        // for( VariableId var : c->GetFreeVariables() )
        //     CheckConsistent( assigns, var );   
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


void SimpleSolver::ReportSolution( const Assignments &assignments, 
                                   shared_ptr<SideInfo> side_info )
{
    map< shared_ptr<Constraint>, list< Value > > vals;
    vals.clear();
    for( shared_ptr<Constraint> c : constraints )
    {
        vals[c] = GetValuesForConstraint(c, assignments);
        ASSERT( vals.at(c).size() == c->GetFreeDegree() ); // Assignments should be complete
    }
    
    holder->ReportSolution( vals, side_info );
}                     


void SimpleSolver::TraceProblem() const
{
    TRACEC("SimpleSolver; %d constraints", constraints.size());
    INDENT(" ");
    for( shared_ptr<Constraint> c : constraints )
    {
        c->TraceProblem();
    }
}


void SimpleSolver::CheckConsistent( const Assignments &assignments, VariableId variable )
{
    ASSERT( assignments.count(variable) > 0 );   // in the assignment
    ::CheckConsistent( variable, assignments.at(variable) );
}


void SimpleSolver::ShowBestAssignment()
{
    if( best_assignments.empty() )
        return; // didn't get around to updating it yet
    TRACE("Best assignment assigned %d of %d variables:\n", best_num_assignments, best_assignments.size());
    INDENT(" ");
    for( VariableId var : variables )
    {
        if( best_assignments.count(var) > 0 )
        {
            TRACEC("Variable ")(*var)(" assigned ")(*best_assignments.at(var));
            if( var->IsLocalMatch(best_assignments.at(var).get()) )
            {
                TRACEC(" is consistent\n");
            }
            else
            {
                TRACEC(" is not consistent (two reasons this might be OK)\n");            
            }
            // Reason 1: At the point we gave up, no constraint containing this 
            // variable was complete (ie had a full set of assigned variables)
            // Reason 2: Complete constraints contained this variable but that
            // didn't include one which had the variable as its base, and so 
            // local match was not enforced.
        }
        else 
        {
            TRACEC("Variable ")(*var)(" could not be matched.\n");
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

