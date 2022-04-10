#include "simple_solver.hpp"
#include "solver_holder.hpp"
#include "query.hpp"
#include "agents/agent.hpp"
#include "the_knowledge.hpp"
#include "../sym/result.hpp"

#include <algorithm>

#define TRACK_BEST_ASSIGNMENT

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
    holder(nullptr)
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

    
void SimpleSolver::Run( ReportageObserver *holder_ )
{
    ASSERT(holder==nullptr)("You can bind a solver to more than one holder, but you obviously can't overlap their Run()s, stupid.");
    ScopedAssign<ReportageObserver *> sa(holder, holder_);
    ASSERT( holder );

    // Do a test with the fully forced constraints (i.e. all vars are forced) with no assignments 
    // (=free variables), so fully forced constraints will be tested. From here on we can test only 
    // constraints affected by changed assignments.
    TRACE("testing\n");
    auto t = ConsistencyCheck( assignments, plan.fully_forced_constraint_set, VariableId() );
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
        holder->ReportSolution( Assignments{} );
    }
    else
    {                
        TRACE("Simple solver matched on forced variables; solving for frees\n");  
        Solve( plan.free_variables.begin() );    
    }

    TRACE("Simple solver ends\n");    
}


void SimpleSolver::Solve( list<VariableId>::const_iterator current_it )
{     
    TRACE("SS%d solving...\n");
    TRACEC("Free vars ")(plan.free_variables)("\n");
    TRACEC("Starting at ")(*current_it)("\n");
    
    // Selector for first variable
    map< VariableId, shared_ptr<ValueSelector> > value_selectors;
    value_selectors[plan.free_variables.front()] = 
        make_shared<ValueSelector>( plan, *this, knowledge, assignments, current_it );
    TRACEC("Made selector for ")(*current_it)("\n");

    while(true)
    {
        Value value;
#ifdef BACKJUMPING
        ConstraintSet unsatisfied;
        tie(value, unsatisfied) = value_selectors.at(*current_it)->SelectNextValue();        
#else        
        value = value_selectors.at(*current_it)->SelectNextValue();        
#endif

        if( !value ) // no consistent value
        {
#ifdef BACKJUMPING
            TRACEC("Inconsistent. Unsatisfied constraints: ")(unsatisfied)("\n");
            set<VariableId> possibly_conflicted_vars = GetAllAffected(unsatisfied);
            TRACEC("Possible conflicted variables: ")(possibly_conflicted_vars)("\n");
#endif
            bool backjump = false;
            do
            {
                value_selectors.erase(*current_it);            
                TRACEC("Killed selector for ")(*current_it)("\n");
                
                if( current_it == plan.free_variables.begin() )
                    goto CEASE; // no more solutions
                --current_it; 
                TRACEC("Back to ")(*current_it)("\n");
                
#ifdef BACKJUMPING
                backjump = (possibly_conflicted_vars.count(*current_it) == 0);
#endif                
                if( backjump )
                    TRACEC("Backjump over ")(*current_it)("\n");
            } while( backjump ); // backjump into possibly_conflicted_vars
        }        
        else
        {
            ++current_it; // try advance
            if( current_it != plan.free_variables.end() ) // new variable
            {
                value_selectors[*current_it] = 
                    make_shared<ValueSelector>( plan, *this, knowledge, assignments, current_it );     
                TRACEC("Advanced to and made selector for ")(*current_it)("\n");
            }
            else // complete
            {
                TRACEC("Reporting solution\n");
                // Engine wants free assignments only, don't annoy it.
                Assignments free_assignments = DifferenceOfSolo( assignments, 
                                                                forced_assignments );
                holder->ReportSolution( free_assignments );
                TRACE("SS%d finished reporting solution\n");
                --current_it;
                TRACEC("Back to ")(*current_it)("\n");                
            }                    
        }
    }        
    CEASE:
    TRACEC("Finished solving\n");
}


SimpleSolver::ValueSelector::ValueSelector( const Plan &solver_plan_,
                                            const SimpleSolver &solver_,
                                            const SR::TheKnowledge *knowledge_,
                                            Assignments &assignments_,
                                            list<VariableId>::const_iterator current_it_ ) :
    solver_plan( solver_plan_ ),
    solver( solver_ ),
    knowledge( knowledge_ ),
    assignments( assignments_ ),
    current_it( current_it_ ),
    current_var( *current_it ),
    constraints_to_test( solver_plan.completed_constraints.at(current_var) )
{
    //ASSERT( current_it != solver_plan.free_variables.end() );
    ASSERT( assignments.count(current_var) == 0 );
    INDENT("V");
       
    bool rok = false;
    shared_ptr<SYM::SymbolSetResult> result; 
    for( shared_ptr<Constraint> c : constraints_to_test )
    {                               
        if( current_var )
        {
            shared_ptr<SYM::SymbolSetResult> r = c->GetSuggestedValues( assignments, current_var );
            if( !rok ) // first successful hint
            {
                result = r;
                rok = true;
            }
            else // subsequent hit should obtain intersection
            {
                result = SYM::SymbolSetResult::GetIntersection({result, r});
            }
        }
    }

    set<Value> s;
    bool sok = false;
    if( rok )
        sok = result->TryGetAsSetOfXLinks(s);
       
    if( rok && sok )
        SetupSuggestionGenerator( s );
    else
        SetupDefaultGenerator();
}

       
SimpleSolver::ValueSelector::~ValueSelector()
{
    assignments.erase(current_var);
}


void SimpleSolver::ValueSelector::SetupDefaultGenerator()
{
    Value start_val = knowledge->depth_first_ordered_domain.front();   
    const SR::TheKnowledge::Nugget &start_nugget( knowledge->GetNugget(start_val) );        
    fwd_it = rev_it = start_nugget.depth_first_ordered_it;
    
    // Forward/backward ordering starting at value of previous variable, prioritizing MMAX.
    go_forward = true;
    insert_mmax_next = knowledge->unordered_domain.count(SR::XLink::MMAX_Link);
    remaining_count = knowledge->depth_first_ordered_domain.size();
    
    values_generator = [this]() -> Value
    {
        Value v;
        if( remaining_count == 0 )
        {
            v = Value();
        }
        else if( insert_mmax_next )
        {
            v = SR::XLink::MMAX_Link; 
            insert_mmax_next = false;
        }
        else
        {
            do
            {
                if( go_forward )
                {
                    v = *fwd_it;
                    AdvanceWithWrap( knowledge->depth_first_ordered_domain, fwd_it, 1 );
                }
                else
                {
                    AdvanceWithWrap( knowledge->depth_first_ordered_domain, rev_it, -1 );
                    v = *rev_it;
                }
                go_forward = !go_forward;
            } while( v == SR::XLink::MMAX_Link );
        }                            
        remaining_count--;      
        return v;          
    };
}


void SimpleSolver::ValueSelector::SetupSuggestionGenerator( set<Value> s )
{
    suggested = s;
    TRACE("At ")(current_var)(", got suggestion ")(suggested)(" - rewriting queue\n"); 
    // Taking hint means new generator that only reveals the hint
    suggestion_iterator = suggested.begin();
    values_generator = [this]() -> Value
    {
        if( suggestion_iterator != suggested.end() )
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
    suggestion_ok = true;
}


SimpleSolver::ValueSelector::SelectNextValueRV SimpleSolver::ValueSelector::SelectNextValue()
{
    INDENT("N");    
    TRACE("Finding value for variable ")(current_var)("\n");
    while(1)
    {       
        Value value = values_generator();
        if( !value )
            break;
        assignments[current_var] = value;
        
        bool ok, sok;
        set<Value> s;        
        ASSERT( solver_plan.completed_constraints.count(current_var) == 1 )
              ("\nfree_variables")(solver_plan.free_variables)
              ("\naffected_constraints:\n")(solver_plan.affected_constraints)
              ("\ncompleted_constraints:\n")(solver_plan.completed_constraints)
              ("\ncurrent_var: ")(current_var);
        Hint new_hint;
#ifdef BACKJUMPING
        ConstraintSet unsatisfied;     
        tie(ok, sok, s, unsatisfied) = solver.ConsistencyCheck( assignments, constraints_to_test, current_var );        
        ASSERT( ok || !unsatisfied.empty() );
#else
        tie(ok, sok, s) = solver.ConsistencyCheck( assignments, constraints_to_test, current_var );        
#endif
   
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
#ifdef BACKJUMPING
    ASSERT( !all_unsatisfied.empty() ); // Note: could fire if domain is empty
    return make_pair(Value(), all_unsatisfied);
#else
    return Value();
#endif
}


SimpleSolver::CCRV SimpleSolver::ConsistencyCheck( const Assignments &assignments,
                                                   const ConstraintSet &to_test,
                                                   const VariableId &current_var ) const 
{
#ifdef BACKJUMPING
    ConstraintSet unsatisfied;
#endif
    bool suggestion_ok = false;
    set<Value> suggested; 
    bool matched = true;
    for( shared_ptr<Constraint> c : to_test )
    {                               
        bool my_matched;
        my_matched = c->IsConsistent(assignments); 
        if( !my_matched )
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
    return make_tuple( matched, suggestion_ok, suggested, unsatisfied );
#else                       
    return make_tuple( matched, suggestion_ok, suggested );
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
