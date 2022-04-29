#include "simple_solver.hpp"
#include "solver_holder.hpp"
#include "query.hpp"
#include "agents/agent.hpp"
#include "the_knowledge.hpp"
#include "../sym/result.hpp"

#include <algorithm>

// Gather stats on the successiness of simple forward checking
#define GATHER_GSV

// Define this to check we don't get as far as trying to make a values generator
// for an empty set (eg because the residual of the domain after forward-checking 
// was empty).
//#define CHECK_NONEMPTY_RESIDUAL

using namespace CSP;

// BACKJUMPING moved to header

SimpleSolver::Plan::Plan( SimpleSolver *algo_,
                          const list< shared_ptr<Constraint> > &constraints_, 
                          const list<VariableId> &free_variables_, 
                          const list<VariableId> &forced_variables_ ) :
    algo( algo_ ), 
    constraints(constraints_),
    free_variables(free_variables_),
    forced_variables(forced_variables_)
{
    DeduceVariables();
} 


void SimpleSolver::Plan::DeduceVariables()
{   
    set<VariableId> free_variables_set;
    completed_constraints.clear();
    for( VariableId v : free_variables )   
    {    
        InsertSolo(free_variables_set, v); // Checks that elements of the list are unique
        (void)completed_constraints[v]; // ensure there is a ConstraintSet for every var even if empty
    }

    set<VariableId> forced_variables_set;
    for( VariableId v : forced_variables )       
        InsertSolo(forced_variables_set, v); // Checks that elements of the list are unique

    ASSERT( IntersectionOf( free_variables_set, forced_variables_set ).empty() );
 
    affected_constraints.clear();
    fully_forced_constraint_set.clear();
 
    set<VariableId> free_variables_used_by_constraints; // For a cross-check
    //TRACE("Constraints:\n")(constraints)("\n");
    for( shared_ptr<Constraint> c : constraints )
    {
        constraint_set.insert(c);
        
        set<VariableId> c_vars = c->GetVariables();
        set<VariableId> c_free_vars;
        for( VariableId v : c_vars )
        {
            if( free_variables_set.count(v) == 1 )
                c_free_vars.insert(v);
        }        
        for( VariableId v : c_free_vars )
        {
            affected_constraints[v].insert(c);
            if( free_variables_used_by_constraints.count(v) == 0 )
                free_variables_used_by_constraints.insert( v );
        }
                        
        if( c_free_vars.empty() )
        {
            fully_forced_constraint_set.insert( c );
        }
        else
        {
            // Which variables complete this contraint
            set<VariableId> cumulative_free_vars;
            for( VariableId v : free_variables )
            {
                cumulative_free_vars.insert(v);
                bool got_all_c_free_vars = true;
                for( VariableId v : c_free_vars )
                    if( cumulative_free_vars.count(v) == 0 )
                        got_all_c_free_vars = false;
                if( got_all_c_free_vars )
                {
                    completed_constraints.at(v).insert(c);
                    break; // we're done - don't insert c for any more vars
                }
            }
        }
        
        free_vars_for_constraint[c] = c_free_vars;
    }
    
    TRACE("Variables supplied by engine: cross-checking\n");
    // Ensure that every free variable supplied to our constructor is
    // used by at least one constraint.
    for( VariableId v : free_variables )
        ASSERT( free_variables_used_by_constraints.count(v) == 1 )
              ("free_variables:\n")(free_variables)
              ("\nfree_variables_used_by_constraints:\n")(free_variables_used_by_constraints); 
}


string SimpleSolver::Plan::GetTrace() const 
{
    return algo->GetName() + ".plan";
}


SimpleSolver::SimpleSolver( const list< shared_ptr<Constraint> > &constraints, 
                            const list<VariableId> &free_variables, 
                            const list<VariableId> &forced_variables ) :
    plan( this, constraints, free_variables, forced_variables ),
    solution_report_function(),
    rejection_report_function()
{
}
                        

void SimpleSolver::Start( const Assignments &forces,
                          const SR::TheKnowledge *knowledge_ )
{
    TRACE("Simple solver begins\n");
    INDENT("S");
    forced_assignments = forces;
    knowledge = knowledge_;

    // Check that the forces passed to us here match the plan
    for( VariableId v : plan.forced_variables )           
        ASSERT( forces.count(v) == 1 );
    ASSERT( forces.size() == plan.forced_variables.size() );

    // Tell all the constraints about the forces
    for( shared_ptr<CSP::Constraint> c : plan.constraints )
        c->Start( knowledge );

    assignments = forced_assignments;    
}

    
void SimpleSolver::Run( const SolutionReportFunction &solution_report_function_,
                        const RejectionReportFunction &rejection_report_function_ )
{
    ASSERT( !solution_report_function )("Something bad like overlapped Run() calls happened.");
    ASSERT( !rejection_report_function )("Something bad like overlapped Run() calls happened.");
    ScopedAssign<SolutionReportFunction> sa1(solution_report_function, solution_report_function_);
    ScopedAssign<RejectionReportFunction> sa2(rejection_report_function, rejection_report_function_);
    ASSERT( solution_report_function );
    // Don't assert on rejection_report_function - it's optional

    // Do a test with the fully forced constraints (i.e. all vars are forced) with no assignments 
    // (=free variables), so fully forced constraints will be tested. From here on we can test only 
    // constraints affected by changed assignments.
    TRACE("testing\n");
    auto t = ConsistencyCheck( assignments, plan.fully_forced_constraint_set );
    TRACE("tested\n");
    
    if( !get<0>(t) )
    {
        TRACE("Simple solver mismatched on forced variables only\n");
        return; // We failed with no assignments, so we cannot match - no solutions will be reported
    }
    
    if( plan.free_variables.empty() )
    {
        TRACE("Simple solver matched on forced variables and no frees\n");  
        // No free vars, so we've got a solution
        solution_report_function( Assignments{} );
    }
    else
    {                
        TRACE("Simple solver matched on forced variables; solving for frees\n");  
        Solve( plan.free_variables.begin() );    
    }

    TRACE("Simple solver ends\n");    
}


void SimpleSolver::Solve( list<VariableId>::const_iterator current_var_it )
{     
    TRACE("SS%d solving...\n");
    TRACEC("Free vars ")(plan.free_variables)("\n");
    TRACEC("Starting at ")(*current_var_it)("\n");
    
    // Selector for first variable    
    value_selectors[plan.free_variables.front()] = 
        make_shared<ValueSelector>( plan, *this, knowledge, assignments, *current_var_it );
    TRACEC("Made selector for ")(*current_var_it)("\n");

#ifdef BACKJUMPING
    conflicted_count = 0;
#endif
    while(true)
    {
        Value value;
#ifdef BACKJUMPING
        ConstraintSet unsatisfied;
        tie(value, unsatisfied) = SelectNextValue(*current_var_it);        
#else        
        value = SelectNextValue(*current_var_it);        
#endif

        if( !value ) // no consistent value
        {
#ifdef BACKJUMPING
            ConstraintSet suspect = plan.affected_constraints.at(*current_var_it); // Was: unsatisfied

            TRACEC("Inconsistent. Possible conflicted constraints: ")(suspect)("\n");
            set<VariableId> possibly_conflicted_vars = GetAllAffected(suspect);
            TRACEC("Possible conflicted variables: ")(possibly_conflicted_vars)("\n");
#endif
            bool backjump = false;
            do
            {
                value_selectors.erase(*current_var_it);            
                TRACEC("Killed selector for ")(*current_var_it)("\n");
                
                if( current_var_it == plan.free_variables.begin() )
                    goto CEASE; // no more solutions
                --current_var_it; 
                TRACEC("Back to ")(*current_var_it)("\n");
                
#ifdef BACKJUMPING
                backjump = ( possibly_conflicted_vars.count(*current_var_it) == 0 &&
                             conflicted_count==0 );
#endif                
                if( backjump )
                    TRACEC("Backjump over ")(*current_var_it)("\n");
            } while( backjump ); // backjump into possibly_conflicted_vars
            
            // Current assignments are belived to be a no-good set so reject
            // them (for a test harness to check).
            if( rejection_report_function )
                rejection_report_function( assignments );
            
#ifdef BACKJUMPING
            conflicted_count++;
#endif
        }        
        else
        {
#ifdef BACKJUMPING
            conflicted_count = 0;
#endif
            ++current_var_it; // try advance
            if( current_var_it != plan.free_variables.end() ) // new variable
            {
                value_selectors[*current_var_it] = 
                    make_shared<ValueSelector>( plan, *this, knowledge, assignments, *current_var_it );     
                TRACEC("Advanced to and made selector for ")(*current_var_it)("\n");
            }
            else // complete
            {
                TRACEC("Reporting solution\n");
                // Engine wants free assignments only, don't annoy it.
                Assignments free_assignments = DifferenceOfSolo( assignments, 
                                                                 forced_assignments );
                solution_report_function( free_assignments );
                TRACE("SS%d finished reporting solution\n");
                --current_var_it;
                TRACEC("Back to ")(*current_var_it)("\n");                
            }                    
        }
    }        
    CEASE:
    TRACEC("Finished solving\n");
}


SimpleSolver::SelectNextValueRV SimpleSolver::SelectNextValue( VariableId my_var )
{
    INDENT("N");    
    TRACE("Finding value for variable ")(my_var)("\n");

    const ConstraintSet &constraints_to_test = plan.completed_constraints.at(my_var);

#ifdef BACKJUMPING
    ConstraintSet all_unsatisfied;     
#endif
    int values_tried_count = 0;

    while( Value value = value_selectors.at(my_var)->GetNextValue() )
    {       
        assignments[my_var] = value;
        
        ASSERT( plan.completed_constraints.count(my_var) == 1 )
              ("\nfree_variables")(plan.free_variables)
              ("\naffected_constraints:\n")(plan.affected_constraints)
              ("\ncompleted_constraints:\n")(plan.completed_constraints)
              ("\ncurrent_var: ")(my_var);
              
        bool ok;
#ifdef BACKJUMPING
        ConstraintSet unsatisfied;     
        tie(ok, unsatisfied) = ConsistencyCheck( assignments, constraints_to_test );        
        ASSERT( ok || !unsatisfied.empty() );
#else
        tie(ok) = ConsistencyCheck( assignments, constraints_to_test );        
#endif

        values_tried_count++;
#ifdef BACKJUMPING
        all_unsatisfied = UnionOf(all_unsatisfied, unsatisfied);
#endif       
        if( ok )
        {
            TRACEC("Value is ")(value)("\n");
#ifdef BACKJUMPING
            return make_pair(value, all_unsatisfied);
#else
            return value;
#endif
        }
    }
    TRACEC("No (more) values found\n");
#ifdef CHECK_NONEMPTY_RESIDUAL
    ASSERT( values_tried_count > 0 ); // Note: could fire if domain is empty
#endif
#ifdef BACKJUMPING
#ifdef CHECK_NONEMPTY_RESIDUAL
    ASSERT( !all_unsatisfied.empty() ); 
#endif
    return make_pair(Value(), all_unsatisfied);
#else
    return Value();
#endif
}


SimpleSolver::ValueSelector::ValueSelector( const Plan &solver_plan_,
                                            const SimpleSolver &solver_,
                                            const SR::TheKnowledge *knowledge_,
                                            Assignments &assignments_,
                                            VariableId var ) :
    solver_plan( solver_plan_ ),
    solver( solver_ ),
    knowledge( knowledge_ ),
    assignments( assignments_ ),
    my_var( var ),
    constraints_to_query( solver_plan.affected_constraints.at(my_var) ),
    constraints_to_test( solver_plan.completed_constraints.at(my_var) )
{
    //ASSERT( current_var_it != solver_plan.free_variables.end() );
    ASSERT( assignments.count(my_var) == 0 );
    INDENT("V");
       
    list<shared_ptr<SYM::SymbolSetResult>> rl; 
    for( shared_ptr<Constraint> c : constraints_to_query )
    {                               
        shared_ptr<SYM::SymbolSetResult> r = c->GetSuggestedValues( assignments, my_var );
        ASSERT( r );
        rl.push_back(r);
    }

    auto s = make_shared<set<Value>>(); // could be unique_ptr in C++14 when we can move-capture
    shared_ptr<SYM::SymbolSetResult> result = SYM::SymbolSetResult::GetIntersection(rl);
    ASSERT( result );
    bool sok = result->TryGetAsSetOfXLinks(*s);
       
#ifdef GATHER_GSV
    gsv_n++;
    if( !sok )
        gsv_nfail++;
    else if( s->empty() )
        gsv_nempty++;
    else
        gsv_tot += s->size();
#endif       
              
    if( sok )
        SetupSuggestionGenerator( s );
    else
        SetupDefaultGenerator();
}

       
SimpleSolver::ValueSelector::~ValueSelector()
{
    assignments.erase(my_var);
}


void SimpleSolver::ValueSelector::SetupDefaultGenerator()
{
    SR::TheKnowledge::DepthFirstOrderedIt fwd_it = knowledge->depth_first_ordered_domain.begin();     
    values_generator = [=]() mutable -> Value
    {
        if( fwd_it==knowledge->depth_first_ordered_domain.end() )        
            return Value();
        
        Value v = *fwd_it;
        ++fwd_it;
        return v;          
    };
}


void SimpleSolver::ValueSelector::SetupSuggestionGenerator( shared_ptr<set<Value>> suggested )
{
#ifdef CHECK_NONEMPTY_RESIDUAL
    ASSERT( !suggested->empty() );
#endif
     // Use of shared_ptr here allows the lambda to keep suggested
     // alive without copying it. Even if we could deal with the slowness of a copy, 
     // we'd still get a crash because the initial suggestion_iterator would be
     // invalid for the copy. Could be unique_ptr in C++14 when we can move-capture
    TRACE("At ")(my_var)(", got suggestion ")(*suggested)(" - rewriting queue\n"); 
    // Taking hint means new generator that only reveals the hint
    set<Value>::iterator suggestion_iterator = suggested->begin();
    values_generator = [=]() mutable -> Value
    {
        if( suggestion_iterator != suggested->end() )
        {                     
            Value v = *suggestion_iterator;
            suggestion_iterator++;
            return v;
        }
        else
        {
            return Value();
        }
    };
}


Value SimpleSolver::ValueSelector::GetNextValue()
{
    // Use the lambda
    return values_generator();
}


void SimpleSolver::ValueSelector::DumpGSV()
{
    FTRACES("Suggestions dump\n");
    FTRACEC("Failure to extensionalise: %f%%\n", 100.0*gsv_nfail/gsv_n);
    FTRACEC("Empty set: %f%%\n", 100.0*gsv_nempty/gsv_n);
    FTRACEC("Average size of successful, non-empty: %f\n", 1.0*gsv_tot/(gsv_n-gsv_nfail-gsv_nempty));
}


uint64_t SimpleSolver::ValueSelector::gsv_n = 0;
uint64_t SimpleSolver::ValueSelector::gsv_nfail = 0;
uint64_t SimpleSolver::ValueSelector::gsv_nempty = 0;
uint64_t SimpleSolver::ValueSelector::gsv_tot = 0;


SimpleSolver::CCRV SimpleSolver::ConsistencyCheck( const Assignments &assignments,
                                                   const ConstraintSet &to_test ) const 
{
#ifdef BACKJUMPING
    ConstraintSet unsatisfied;
#endif
    bool matched = true;
    for( shared_ptr<Constraint> c : to_test )
    {                               
        if( !c->IsConsistent(assignments) )
        {            
            matched = false;
#ifdef BACKJUMPING
            unsatisfied.insert( c );
#else
            break;
#endif
        }
    } 
#ifdef BACKJUMPING
#ifdef CHECK_NONEMPTY_RESIDUAL
    if( !matched )
        ASSERT( !unsatisfied.empty() );
#endif        
    return make_tuple( matched, unsatisfied );
#else                       
    return make_tuple( matched );
#endif                       
}


void SimpleSolver::ShowBestAssignment()
{
    Assignments &assignments_to_show = assignments;
    INDENT("B");
    if( assignments_to_show.empty() )
        return; // didn't get around to updating it yet
    TRACE("FREE VARIABLES: assigned %d of %d:\n", assignments_to_show.size(), plan.free_variables.size());
    for( VariableId var : plan.free_variables )
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
                ASSERT(assignments_to_show.size() <= plan.free_variables.size())("local mismatch in passing complete assignment");
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


void SimpleSolver::CheckPlan() const
{
    set<VariableId> variables_used;
    for( shared_ptr<Constraint> c : plan.constraints )
    {
        set<VariableId> cfv = plan.free_vars_for_constraint.at(c);
        for( VariableId v : cfv )
        {
            ASSERT( find( plan.free_variables.begin(), plan.free_variables.end(), v ) != plan.free_variables.end() )
                  ("Planning error: ")(c)(" is missing variables\n")
                  ("expecting ")(cfv)("\n")
                  ("provided ")(plan.free_variables)("\n");
            variables_used.insert( v );
        }
    }
    for( VariableId v : plan.free_variables )
    {
        ASSERT( variables_used.count(v) > 0 )
              ("Planning error: variable ")(v)(" is not used by any constraints\n")
              ("Variables used: ")(variables_used)("\n");
    }
}


set<VariableId> SimpleSolver::GetAllAffected( ConstraintSet constraints )
{
    set<VariableId> all_vars;
    for( shared_ptr<Constraint> c : constraints )
        all_vars = UnionOf(all_vars, plan.free_vars_for_constraint.at(c));
    return all_vars;            
} 


void SimpleSolver::Dump() const
{
    TRACE("%d constraints:\n", plan.constraints.size());
    for( shared_ptr<Constraint> c : plan.constraints )    
        c->Dump();   
    TRACEC("%d free variables:\n", plan.free_variables.size())(plan.free_variables)("\n");        
}


void SimpleSolver::DumpGSV()
{
    SimpleSolver::ValueSelector::DumpGSV();
}
