#include "reference_solver.hpp"
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

ReferenceSolver::Plan::Plan( ReferenceSolver *algo_,
                             const list< shared_ptr<Constraint> > &constraints_, 
                             const vector<VariableId> &free_variables_, 
                             const set<VariableId> &domain_forced_variables_, 
                             const set<VariableId> &arbitrary_forced_variables_ ) :
    algo( algo_ ), 
    constraints(constraints_),
    free_variables(free_variables_),
    domain_forced_variables(domain_forced_variables_),
    arbitrary_forced_variables(arbitrary_forced_variables_)
{
    DeduceVariables();
} 


ReferenceSolver::~ReferenceSolver()
{
}


void ReferenceSolver::Plan::DeduceVariables()
{   
    set<VariableId> free_variables_set;
    free_variables_to_indices.clear();
    for( int i=0; i<free_variables.size(); i++ )   
    {    
        const VariableId &v = free_variables.at(i);
        InsertSolo(free_variables_set, v); // Checks free vars are unique
        free_variables_to_indices[v] = i;
    }
    
    // Checks that forced vars are unique and disjoint
    set<VariableId> forced_variables = UnionOfSolo( domain_forced_variables, 
                                                    arbitrary_forced_variables );

    // Free and forced sets must be disjoint
    ASSERT( IntersectionOf( free_variables_set, forced_variables ).empty() );
 
    completed_constraints.resize( free_variables.size() );
    affected_constraints.resize( free_variables.size() );
    fully_forced_constraint_set.clear();
 
    set<int> free_variables_used_by_constraints; // For a cross-check
    //TRACE("Constraints:\n")(constraints)("\n");
    for( shared_ptr<Constraint> c : constraints )
    {
        constraint_set.insert(c);
        
        set<VariableId> c_vars = c->GetVariables();
        set<VariableId> c_free_vars;
        set<int> c_free_var_indices;
        for( VariableId v : c_vars )
        {
            if( free_variables_set.count(v) == 1 )
            {
                c_free_vars.insert(v);
                c_free_var_indices.insert( free_variables_to_indices.at(v) );
            }
            if( arbitrary_forced_variables.count(v) == 1 )
            {
                // Enforce rule #525 - the arbitrary forces can be outside the
                // domain and won't have knowledge nuggets. This is OK as long
                // as constraints that have to deal with them don't need nuggets.
                ASSERT( c->GetVariablesRequiringNuggets().count(v) == 0 )
                      ( "Constraint:\n")(c)("\nrequires NUGGETS but ")(v)(" is arbitrary\n");
            }
        }        
        for( int i : c_free_var_indices )
        {
            affected_constraints[i].insert(c);
            if( free_variables_used_by_constraints.count(i) == 0 )
                free_variables_used_by_constraints.insert(i);
        }
                        
        if( c_free_vars.empty() )
        {
            fully_forced_constraint_set.insert( c );
        }
        else
        {
            // Which variables complete this contraint
            set<int> cumulative_free_vars;
            for( int i=0; i<free_variables.size(); i++ )      
            {
                cumulative_free_vars.insert(i);
                bool got_all_c_free_vars = true;
                for( int j : c_free_var_indices )
                    if( cumulative_free_vars.count(j) == 0 )
                        got_all_c_free_vars = false;
                if( got_all_c_free_vars )
                {
                    completed_constraints.at(i).insert(c);
                    break; // we're done - don't insert c for any more vars
                }
            }
        }
        
        free_var_indices_for_constraint[c] = c_free_var_indices;
    }
    
    TRACE("Variables supplied by engine: cross-checking\n");
    // Ensure that every free variable supplied to our constructor is
    // used by at least one constraint.
    for( int i=0; i<free_variables.size(); i++ )
        ASSERT( free_variables_used_by_constraints.count(i) == 1 )
              ("free_variables:\n")(free_variables)
              ("\nfree_variables_used_by_constraints:\n")(free_variables_used_by_constraints); 
}


string ReferenceSolver::Plan::GetTrace() const 
{
    return algo->GetName() + ".plan";
}


ReferenceSolver::ReferenceSolver( const list< shared_ptr<Constraint> > &constraints, 
                                  const vector<VariableId> &free_variables, 
                                  const set<VariableId> &domain_forced_variables, 
                                  const set<VariableId> &arbitrary_forced_variables ) :
    plan( this, constraints, free_variables, domain_forced_variables, arbitrary_forced_variables ),
    solution_report_function(),
    rejection_report_function()
{
}
                        

void ReferenceSolver::Start( const Assignments &forces,
                             const SR::TheKnowledge *knowledge_ )
{
    TRACE("Simple solver begins\n");
    INDENT("S");
    forced_assignments = forces;
    knowledge = knowledge_;

    // Check that the forces passed to us here match the plan
    for( VariableId v : plan.domain_forced_variables )           
        ASSERT( forces.count(v) == 1 );
    for( VariableId v : plan.arbitrary_forced_variables )           
        ASSERT( forces.count(v) == 1 );
    ASSERT( forces.size() == plan.domain_forced_variables.size()+
                             plan.arbitrary_forced_variables.size() );

    // Tell all the constraints about the forces
    for( shared_ptr<CSP::Constraint> c : plan.constraints )
        c->Start();

    assignments = forced_assignments;    
}

    
void ReferenceSolver::Run( const SolutionReportFunction &solution_report_function_,
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
        // Current assignments are believed to be a no-good set so reject
        // them (for a test harness to check).
        if( rejection_report_function )
            rejection_report_function( Assignments{} );
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
        Solve();    
    }

    TRACE("Simple solver ends\n");    
}


void ReferenceSolver::Solve()
{     
    TRACE("SS%d solving...\n");
    TRACEC("Free vars ")(plan.free_variables)("\n");
    TRACEC("Starting at X")(current_var_index)("\n");
    
    // Selector for first variable    
    current_var_index = 0;
    value_selectors[current_var_index] = 
        make_shared<ValueSelector>( plan.affected_constraints.at(current_var_index), 
                                    knowledge, 
                                    assignments, 
                                    plan.free_variables.at(current_var_index) );
    success_count[current_var_index] = 0; 
    TRACEC("Made selector for X")(current_var_index)("\n");

    while(true)
    {
        Value value;
        ConstraintSet unsatisfied;
        tie(value, unsatisfied) = TryFindNextConsistentValue(current_var_index);        

        if( !value ) // no consistent value
        {
            bool cease = AssignUnsuccessful();
          
            // Current assignments are believed to be a no-good set so reject
            // them (for a test harness to check).
            if( rejection_report_function )
                rejection_report_function( assignments );

            if( cease )
                break;
        }        
        else
        {
            AssignSuccessful();
        }
    }        
    CEASE:
    TRACEC("Finished solving\n");
}


void ReferenceSolver::AssignSuccessful()
{
    success_count.at(current_var_index)++;
    current_var_index++;
    if( current_var_index < plan.free_variables.size() ) // new variable
    {
        value_selectors[current_var_index] = 
            make_shared<ValueSelector>( plan.affected_constraints.at(current_var_index), 
                                        knowledge, 
                                        assignments, 
                                        plan.free_variables.at(current_var_index) );     
        success_count[current_var_index] = 0;
        TRACEC("Advanced to and made selector for X")(current_var_index)("\n");
    }
    else // complete
    {
        TRACEC("Reporting solution\n");
        // Engine wants free assignments only, don't annoy it.
        Assignments free_assignments = DifferenceOfSolo( assignments, 
                                                         forced_assignments );
        solution_report_function( free_assignments );
        current_var_index--;
        TRACEC("Back to X")(current_var_index)("\n");                
    }                    
}


bool ReferenceSolver::AssignUnsuccessful()
{
    value_selectors.erase(current_var_index);   
    assignments.erase( plan.free_variables.at(current_var_index) );         
    TRACEC("Killed selector for X")(current_var_index)("\n");
    
    if( current_var_index == 0 )
        return true; // no more solutions
        
    current_var_index--; 
    TRACEC("Back to X")(current_var_index)("\n");
                      
    return false;
}


ReferenceSolver::SelectNextValueRV ReferenceSolver::TryFindNextConsistentValue( int my_var_index )
{
    INDENT("N");    
    TRACE("Finding value for variable X")(my_var_index)("\n");
    
    const ConstraintSet &constraints_to_test = plan.completed_constraints.at(my_var_index);

    ConstraintSet all_unsatisfied;     
    int values_tried_count = 0;

    while( Value value = value_selectors.at(my_var_index)->GetNextValue() )
    {       
        assignments[plan.free_variables.at(my_var_index)] = value;
              
        bool consistent;
        ConstraintSet unsatisfied;     
        tie(consistent, unsatisfied) = ConsistencyCheck( assignments, constraints_to_test );        
        ASSERT( consistent || !unsatisfied.empty() );

        values_tried_count++;
        all_unsatisfied = UnionOf(all_unsatisfied, unsatisfied);      
        if( consistent )
        {
            TRACEC("Value is ")(value)("\n");
            return make_pair(value, all_unsatisfied);
        }
    }
    TRACEC("No (more) values found\n");
#ifdef CHECK_NONEMPTY_RESIDUAL
    ASSERT( values_tried_count > 0 ); // Note: could fire if domain is empty
    ASSERT( !all_unsatisfied.empty() ); 
#endif
    return make_pair(Value(), all_unsatisfied);
}


ReferenceSolver::CCRV ReferenceSolver::ConsistencyCheck( const Assignments &assignments,
                                                         const ConstraintSet &to_test ) const 
{
    ConstraintSet unsatisfied;
    bool matched = true;
    for( shared_ptr<Constraint> c : to_test )
    {                               
        if( !c->IsSatisfied(assignments) )
        {            
            matched = false;
            unsatisfied.insert( c );
        }
    } 
#ifdef CHECK_NONEMPTY_RESIDUAL
    if( !matched )
        ASSERT( !unsatisfied.empty() );
#endif        
    return make_tuple( matched, unsatisfied );                      
}


void ReferenceSolver::ShowBestAssignment()
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


void ReferenceSolver::TimedOperations()
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


void ReferenceSolver::CheckPlan() const
{
    set<VariableId> variables_used;
    for( shared_ptr<Constraint> c : plan.constraints )
    {
        set<int> cfv = plan.free_var_indices_for_constraint.at(c);
        for( int i : cfv )
        {
            variables_used.insert( plan.free_variables.at(i) );
        }
    }
    for( VariableId v : plan.free_variables )
    {
        ASSERT( variables_used.count(v) > 0 )
              ("Planning error: variable ")(v)(" is not used by any constraints\n")
              ("Variables used: ")(variables_used)("\n");
    }
}


set<int> ReferenceSolver::GetAffectedVariableIndices( ConstraintSet constraints )
{
    set<int> all_var_indices;
    for( shared_ptr<Constraint> c : constraints )
        all_var_indices = UnionOf(all_var_indices, plan.free_var_indices_for_constraint.at(c));
    return all_var_indices;            
} 


void ReferenceSolver::Dump() const
{
    TRACE("%d constraints:\n", plan.constraints.size());
    for( shared_ptr<Constraint> c : plan.constraints )    
        c->Dump();   
    TRACEC("%d free variables:\n", plan.free_variables.size())(plan.free_variables)("\n");        
}


void ReferenceSolver::DumpGSV()
{
    ValueSelector::DumpGSV();
}
