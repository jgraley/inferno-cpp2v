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
        TRACE("Variables not supplied by engine: using own");
        // No variables list was supplied, so return the one we generated
        variables = my_variables;
    }
    else
    {
        TRACE("Variables supplied by engine: cross-checking");
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

    set<VariableId> variables_used;
    for( shared_ptr<Constraint> c : plan.constraints )
    {
        list<VariableId> cfv = c->GetFreeVariables();
        for( VariableId v : cfv )
        {
            ASSERT( find( plan.variables.begin(), plan.variables.end(), v ) != plan.variables.end() )
                  ("Planning error: ")(c)(" is missing variables\n")
                  ("expecting ")(cfv)("\n")
                  ("provided ")(plan.variables)("\n");
            variables_used.insert( v );
        }
    }
    for( VariableId v : plan.variables )
    {
        ASSERT( variables_used.count(v) > 0 )
              ("Planning error: variable ")(v)(" is not used by any constraints\n")
              ("Variables used: ")(variables_used)("\n");
    }
}
                        

void SimpleSolver::Run( ReportageObserver *holder_, 
                        const unordered_set< Value > &initial_domain_,
                        const Assignments &forces,
                        const SR::TheKnowledge *knowledge )
{
    INDENT("S");
    ASSERT(holder==nullptr)("You can bind a solver to more than one holder, but you obviously can't overlap their Run()s, stupid.");
    holder = holder_;
    initial_domain = initial_domain_;

    // Tell all the constraints about the forces
    for( shared_ptr<CSP::Constraint> c : plan.constraints )
        c->Start( forces, knowledge );
        
    assignments.clear();
    
#ifdef TRACK_BEST_ASSIGNMENT    
    best_assignments.clear();
    best_num_assignments = 0;
#endif    

    if( plan.variables.empty() )
    {
        // TODO re-organise (and rename) TryVariable() so we don't need this bit
        bool ok = Test( assignments );
        if( ok )
            holder->ReportSolution( assignments );
    }
    else
    {
        try_counts.clear();
        TryVariable( plan.variables.begin() );    
    }

    holder = nullptr;
}


bool SimpleSolver::TryVariable( list<VariableId>::const_iterator current_it )
{
    ASSERT( current_it != plan.variables.end() );
    
    list<VariableId>::const_iterator next_it = current_it;
    ++next_it;
    bool complete = (next_it == plan.variables.end());
    
    int i=0;
    for( Value v : initial_domain )
    {
        assignments[*current_it] = v;
        try_counts[*current_it] = i;
     
        bool ok = Test( assignments );
        
        if( !ok )
        {
            assignments.erase(*current_it); // remove failed variable 
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
        {
#ifdef TRACK_BEST_ASSIGNMENT    
            TRACE("Reporting CSP solution\n");
            ShowBestAssignment();
#endif            
            holder->ReportSolution( assignments );
        }
        else
        {
            TryVariable( next_it );
        }
        i++;
    }

    return false;
}


bool SimpleSolver::Test( const Assignments &assigns )
{
    bool ok = true; // AND-rule    
    report = "";
    for( shared_ptr<Constraint> c : plan.constraints )
    {      
        report += Trace(*c);
        int requirements_met = 0;
        list<VariableId> required_vars = c->GetRequiredVariables();
        for( VariableId rv : required_vars )
            if( assignments.count(rv) > 0 )
                requirements_met++;
           
        if( requirements_met < required_vars.size() )
        {
            report += SSPrintf(" rmet=%d ABORT\n", requirements_met);
            continue;
        }
        
        int free_met = 0;
        list<VariableId> free_vars = c->GetFreeVariables();
        for( VariableId rv : free_vars )
            if( assignments.count(rv) > 0 )
                free_met++;
        report += SSPrintf(" fmet=%d", free_met);
        
        ok = c->Test(assignments); 
        
        if( !ok )
        {
            report += SSPrintf(" MISS\n");
            break;
        }
        report += SSPrintf(" HIT\n");
                            
        // Only expected to pass for base (which we only know about if 
        // it's FREE). We don't check whether the other 
        // values accepted by the constraint are compatible with the 
        // variable's deduced type, because that is the job of the 
        // constraint that is based upon it.
        // for( VariableId var : c->GetFreeVariables() )
        //     CheckLocalMatch( assigns, var );   
    } 
    TimedOperations(); // overhead should be hidden by Constraint::Test()

    return ok;
}


void SimpleSolver::TraceProblem() const
{
    INDENT("T");

    TRACE("SimpleSolver; %d constraints:", plan.constraints.size());
    for( shared_ptr<Constraint> c : plan.constraints )    
        c->TraceProblem();   
    TRACEC("%d variables:", plan.variables.size());        
    for( VariableId var : plan.variables )
        TRACEC(var)("\n");
}


void SimpleSolver::CheckLocalMatch( const Assignments &assignments, VariableId variable )
{
    ASSERT( assignments.count(variable) > 0 );   // in the assignment
    ::CheckLocalMatch( variable, assignments.at(variable) );
}


void SimpleSolver::ShowBestAssignment()
{
    Assignments &assignments_to_show = assignments;
    INDENT("B");
    if( assignments_to_show.empty() )
        return; // didn't get around to updating it yet
    TRACE("VARIABLES: assigned %d of %d:\n", assignments_to_show.size(), plan.variables.size());
    for( VariableId var : plan.variables )
    {
        if( try_counts.count(var) > 0 )
            TRACEC("[%03d] ", try_counts.at(var));
        TRACEC(var);
        if( assignments_to_show.count(var) > 0 )
        {
            TRACE(" assigned ")(assignments_to_show.at(var));
            if( var.GetChildAgent()->IsLocalMatch(assignments_to_show.at(var).GetChildX().get()) || 
                assignments_to_show.at(var) == SR::XLink::MMAX_Link )
            {
                TRACEC(" is a local match\n");
            }
            else
            {
                TRACEC(" is not a local match (two reasons this might be OK)\n");            
                ASSERT(assignments_to_show.size() <= plan.variables.size())("local mismatch in passing complete assignment");
            }
            // Reason 1: At the point we gave up, no constraint containing this 
            // variable had all of its required variables assigned.
            // Reason 2: Complete constraints contained this variable but that
            // didn't include one which had the variable as its base, and so 
            // local match was not enforced.
        }
        else
        {
            TRACEC(" could not be assigned a consistent value.\n");
            break; // solver gives up
        }
    }
    TRACEC("CONSTRAINTS:\n");
    TRACEC(report);
}


void SimpleSolver::TimedOperations()
{
    auto now = chrono::steady_clock::now();
    auto since = now - last_report;
    if( since > chrono::seconds(3) )
    {
        TRACE("Intermediate CSP snapshot\n");
        ShowBestAssignment();
        last_report = now;
    }
}

