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
        constraint_set.insert(c);
        list<VariableId> vars = c->GetFreeVariables();
        
        for( VariableId v : vars )
        {
            if( variables_check_set.count(v) == 0 )
            {
                variables_check_set.insert( v );
                if( variables_ == nullptr )
                    my_variables.push_back( v ); 
            }
            affected_constraints[v].insert(c);
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
    CheckPlanVariablesUsed();
}
                        

void SimpleSolver::Run( ReportageObserver *holder_, 
                        const unordered_set< Value > &initial_domain_,
                        const Assignments &forces,
                        const SR::TheKnowledge *knowledge )
{
    TRACE("Simple solver begins\n");
    INDENT("S");
    ASSERT(holder==nullptr)("You can bind a solver to more than one holder, but you obviously can't overlap their Run()s, stupid.");
    ScopedAssign<ReportageObserver *> sa(holder, holder_);
    ASSERT( holder );
    initial_domain = initial_domain_;

    // Tell all the constraints about the forces
    for( shared_ptr<CSP::Constraint> c : plan.constraints )
        c->Start( forces, knowledge );
        
    assignments.clear();
    
#ifdef TRACK_BEST_ASSIGNMENT    
    best_assignments.clear();
    best_num_assignments = 0;
#endif    
    try_counts.clear();
    
    try
    {
        Test( assignments, plan.constraint_set );
    }
    catch( const ::Mismatch &e )
    {
        return; // We failed with no assignments, so we cannot match
    }        

    if( plan.variables.empty() )
    {
        holder->ReportSolution( assignments );
    }
    else
    {        
        TryVariable( plan.variables.begin() );    
    }

    TRACE("Simple solver ends\n");    
}


bool SimpleSolver::TryVariable( list<VariableId>::const_iterator current_it )
{
    INDENT("T");
    ASSERT( current_it != plan.variables.end() );
   
    list<VariableId>::const_iterator next_it = current_it;
    ++next_it;
    bool complete = (next_it == plan.variables.end());
    bool took_hint = false; 
    int i=0;
    Assignment hint;
    list<Value> value_queue;
    for( Value v : initial_domain )
        value_queue.push_back(v);
    while( !value_queue.empty() )
    {
        Value v = value_queue.front();
        value_queue.pop_front();
        
        TRACE("Trying variable ")(*current_it)(" := ")(v)("\n");
        assignments[*current_it] = v;
        try_counts[*current_it] = i;
        bool ok = false;
        
        tie(ok, hint) = TestNoThrow( assignments, plan.affected_constraints.at(*current_it) );        
        
#ifdef HINTS_IN_EXCEPTIONS   
        if( hint && *current_it==(VariableId)(hint) ) // does have a hint, and for the current variable
        {
            TRACE("At ")(*current_it)(", got hint ")(hint)(" - retesting\n"); 
            assignments[*current_it] = (Value)(hint); // take the hint
            tie(ok, ignore) = TestNoThrow( assignments, plan.affected_constraints.at(*current_it) );      
            took_hint = true;
        }
#endif
        
        if( !ok )
        {
            if( took_hint )
                break;
            else
                continue; // try next value
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
//            ShowBestAssignment();
#endif            
            holder->ReportSolution( assignments );
        }
        else
        {
            TryVariable( next_it );
        }
        
        if( took_hint )
            break; // hint was the only valid value for this variable, given the other assignments
        
        i++;
    }
    
    // Need to do this to avoid old assignments hanging around, being 
    // picked up by partial NLQs and causing mismatches that don't get
    // resolved beacuse this assignment won't be getting incremented.
    assignments.erase(*current_it);

    return false;
}


pair<bool, Assignment> SimpleSolver::TestNoThrow( const Assignments &assigns, const ConstraintSet &to_test )
{
    try
    {
        Test( assignments, to_test );
        return make_pair(true, Assignment());
    }
    catch( const ::Mismatch &e )
    {            
#ifdef HINTS_IN_EXCEPTIONS   
        if( auto pae = dynamic_cast<const SR::Agent::Mismatch *>(&e) ) // could have a hint            
            return make_pair(false, pae->hint);
#endif
        return make_pair(false, Assignment());
    }
}


void SimpleSolver::Test( const Assignments &assigns, const ConstraintSet &to_test )
{
    //TimedOperations(); // overhead should be hidden by Constraint::Test()

    report = "";
    for( shared_ptr<Constraint> c : to_test )
    {      
        TRACE_TO(report)(*c);
            
        int requirements_met = 0;
        list<VariableId> required_vars = c->GetRequiredVariables();
        for( VariableId rv : required_vars )
            if( assignments.count(rv) > 0 )
                requirements_met++;
           
        if( requirements_met < required_vars.size())
            TRACE_TO(report)(" rmet=%d SKIP\n", requirements_met);        
           
        if( requirements_met < required_vars.size() )
            continue;        
        
        if( Tracer::IsEnabled() )
        {
            int free_met = 0;
            list<VariableId> free_vars = c->GetFreeVariables();
            for( VariableId rv : free_vars )
                if( assignments.count(rv) > 0 )
                    free_met++;
            TRACE_TO(report)(" fmet=%d", free_met);
        }
        
        c->Test(assignments); 
                                    
        // Only expected to pass for base (which we only know about if 
        // it's FREE). We don't check whether the other 
        // values accepted by the constraint are compatible with the 
        // variable's deduced type, because that is the job of the 
        // constraint that is based upon it.
        // for( VariableId var : c->GetFreeVariables() )
        //     CheckLocalMatch( assigns, var );   
    } 
}


void SimpleSolver::TraceProblem() const
{
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


void SimpleSolver::CheckPlanVariablesUsed()
{
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
