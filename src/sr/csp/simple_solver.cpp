#include "simple_solver.hpp"
#include "solver_holder.hpp"
#include "query.hpp"
#include "agents/agent.hpp"
#include "the_knowledge.hpp"

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

    assignment_tester = 
        [=]( const Assignments &assigns, const ConstraintSet &to_test ) -> pair<bool, Assignment>
    {
        return Test( assigns, to_test );
    };    
}
                        

void SimpleSolver::Run( ReportageObserver *holder_, 
                        const Assignments &forces,
                        const SR::TheKnowledge *knowledge_ )
{
    TRACE("Simple solver begins\n");
    INDENT("S");
    knowledge = knowledge_;
    ASSERT(holder==nullptr)("You can bind a solver to more than one holder, but you obviously can't overlap their Run()s, stupid.");
    ScopedAssign<ReportageObserver *> sa(holder, holder_);
    ASSERT( holder );

    // Tell all the constraints about the forces
    for( shared_ptr<CSP::Constraint> c : plan.constraints )
        c->Start( forces, knowledge );
        
    assignments.clear();    
    
    bool ok;
    Assignment hint;  
    // Do a test with all constraints but no assignments (=free variables), so forced variables 
    // will be tested. From here on we can test only constraints affected by changed assignments.
    tie(ok, hint) = Test( assignments, plan.constraint_set );
    
    if( !ok )
        return; // We failed with no assignments, so we cannot match - no solutions will be reported
    
    if( plan.variables.empty() )
    {
        // No free vars, so we've got a solution
        holder->ReportSolution( assignments );
    }
    else
    {        
        // Work through the free vars
        value_selectors[plan.variables.front()] = 
            make_shared<ValueSelector>( plan, assignment_tester, knowledge, assignments, plan.variables.begin() );
        
        TryVariable( plan.variables.begin() );    
    }

    TRACE("Simple solver ends\n");    
}


#define DERECURSE

void SimpleSolver::TryVariable( list<VariableId>::const_iterator current_it )
{     
    while(true)
    {
        if( !value_selectors.at(*current_it)->SelectNextValue() )
        {
            value_selectors[*current_it] = nullptr;
            if( current_it == plan.variables.begin() )
                return;
#ifdef DERECURSE
            --current_it; // backtrack
            continue;
#else            
            break; // backtrack
#endif            
        }
                    
        ++current_it; // try advance
        if( current_it == plan.variables.end() ) // complete
        {
            holder->ReportSolution( assignments );
            --current_it;
            continue;
        }
        
        value_selectors[*current_it] = 
            make_shared<ValueSelector>( plan, assignment_tester, knowledge, assignments, current_it );
#ifndef DERECURSE
        TryVariable( current_it ); 
        --current_it;
#endif        
    }        
}


SimpleSolver::ValueSelector::ValueSelector( const Plan &solver_plan_,
                                            const AssignmentTester &assignment_tester_,
                                            const SR::TheKnowledge *knowledge_,
                                            Assignments &assignments_,
                                            list<VariableId>::const_iterator current_it_ ) :
    solver_plan( solver_plan_ ),
    assignment_tester( assignment_tester_ ),
    knowledge( knowledge_ ),
    assignments( assignments_ ),
    current_it( current_it_ ),
    current_var( *current_it )
{
    ASSERT( current_it != solver_plan.variables.end() );
    INDENT("T");
       
    Value start_val;
    if( current_it==solver_plan.variables.begin() )
    {
        start_val = knowledge->ordered_domain.front();
    }
    else
    {
        list<VariableId>::const_iterator prev_it = current_it;
        --prev_it; 
        start_val = assignments[*prev_it];   
    }
    
    const SR::TheKnowledge::Nugget &nugget( knowledge->GetNugget(start_val) );        
    SR::TheKnowledge::Nugget::OrderedIt fwd_it = nugget.ordered_it;
    SR::TheKnowledge::Nugget::OrderedIt rev_it = nugget.ordered_it;
    
    // Forward/backward ordering starting at value of previous variable, prioritizing MMAX.
    bool go_forward = true;
    for( Value v_unused : knowledge->ordered_domain ) // just to get the right number of iterations
    {
        Value v;
        if( go_forward )
        {
            v = *fwd_it;
            AdvanceWithWrap( knowledge->ordered_domain, fwd_it, 1 );
        }
        else
        {
            AdvanceWithWrap( knowledge->ordered_domain, rev_it, -1 );
            v = *rev_it;
        }
        if( v == SR::XLink::MMAX_Link )
            value_queue.push_front(v);
        else
            value_queue.push_back(v);
        go_forward = !go_forward;
    }
}


SimpleSolver::ValueSelector::~ValueSelector()
{
    // Need to do this to avoid old assignments hanging around, being 
    // picked up by partial NLQs and causing mismatches that don't get
    // resolved beacuse this assignment won't be getting incremented.
    assignments.erase(current_var);
}


Value SimpleSolver::ValueSelector::SelectNextValue()
{
    while( !value_queue.empty() )
    {       
        TRACE("Trying variable ")(current_var)(" := ")(value_queue.front())("\n");
        Value value = value_queue.front();
        assignments[current_var] = value;
        value_queue.pop_front();
        
        bool ok;
        Assignment hint;        
        tie(ok, hint) = assignment_tester( assignments, solver_plan.affected_constraints.at(current_var) );        
        
        if( !ok && hint && current_var==(VariableId)(hint) ) // we got a hint, and for the current variable
        {
            TRACE("At ")(current_var)(", got hint ")(hint)(" - rewriting queue\n"); 
            value_queue.clear();
            value_queue.push_back( (Value)(hint) ); 
        }
       
        if( !ok )
           continue; // try next value
           
        return value;
    }
    return Value();
}


pair<bool, Assignment> SimpleSolver::Test( const Assignments &assigns, const ConstraintSet &to_test ) const 
{
    ConstraintSet unsatisfied;
    list<Assignment> hints;
    for( shared_ptr<Constraint> c : to_test )
    {                  
        int requirements_met = 0;
        list<VariableId> required_vars = c->GetRequiredVariables();
        for( VariableId rv : required_vars )
            if( assigns.count(rv) > 0 )
                requirements_met++;
           
        if( requirements_met < required_vars.size() )
            continue;        
        
        try
        {
            c->Test(assigns); 
        }
        catch( const ::Mismatch &e )
        {            
#ifdef HINTS_IN_EXCEPTIONS   
            if( auto pae = dynamic_cast<const SR::Agent::Mismatch *>(&e) ) // could have a hint            
                hints.push_back( pae->hint );
#endif
            unsatisfied.insert( c );
        }
       
        // Only expected to pass for base (which we only know about if 
        // it's FREE). We don't check whether the other 
        // values accepted by the constraint are compatible with the 
        // variable's deduced type, because that is the job of the 
        // constraint that is based upon it.
        // for( VariableId var : c->GetFreeVariables() )
        //     CheckLocalMatch( assigns, var );   
    } 
    return make_pair( unsatisfied.empty(), 
                      hints.empty() ? Assignment() : hints.front() );
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


void SimpleSolver::CheckLocalMatch( const Assignments &assigns, VariableId variable )
{
    ASSERT( assigns.count(variable) > 0 );   // in the assignment
    ::CheckLocalMatch( variable, assigns.at(variable) );
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
