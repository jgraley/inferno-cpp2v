#include "simple_solver.hpp"
#include "solver_holder.hpp"
#include "query.hpp"
#include "agents/agent.hpp"
#include "the_knowledge.hpp"

#include <algorithm>

#define TRACK_BEST_ASSIGNMENT

using namespace CSP;


//#define BACKJUMPING
#define TAKE_HINTS
#define DYNAMIC_START_VALUE

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
        TRACE("Variables not supplied by engine: using own\n");
        // No variables list was supplied, so return the one we generated
        variables = my_variables;
    }
    else
    {
        TRACE("Variables supplied by engine: cross-checking\n");
        // A variables list was supplied and it must have the same set of variables
        for( VariableId v : *variables_ )
            ASSERT( variables_check_set.count(v) == 1 );
        variables = *variables_;
    }
}


SimpleSolver::SimpleSolver( const list< shared_ptr<Constraint> > &constraints_, 
                            const list<VariableId> *variables_ ) :
    plan( constraints_, variables_ ),
    holder(nullptr),
    my_index( next_index++ )
{
    TraceProblem();
    CheckPlanVariablesUsed();    
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
    
    // Do a test with all constraints but no assignments (=free variables), so forced variables 
    // will be tested. From here on we can test only constraints affected by changed assignments.
    auto t = Test( assignments, plan.constraint_set );
    
    if( !get<0>(t) )
        return; // We failed with no assignments, so we cannot match - no solutions will be reported
    
    if( plan.variables.empty() )
    {
        // No free vars, so we've got a solution
        holder->ReportSolution( assignments );
    }
    else
    {                
        Solve( plan.variables.begin() );    
    }

    TRACE("Simple solver ends\n");    
}


void SimpleSolver::Solve( list<VariableId>::const_iterator current_it )
{     
    TRACE("SS%d solving...\n");
    TRACEC("Vars ")(plan.variables)("\n");
    TRACEC("Starting at ")(*current_it)("\n");
    
    // Selector for first variable
    map< VariableId, shared_ptr<ValueSelector> > value_selectors;
    value_selectors[plan.variables.front()] = 
        make_shared<ValueSelector>( plan, *this, knowledge, assignments, current_it );
    TRACEC("Made selector for ")(*current_it)("\n");

    while(true)
    {
        auto p = value_selectors.at(*current_it)->SelectNextValue();        
        if( !p.first ) // no consistent value
        {
            ConstraintSet unsatisfied = p.second;
            TRACEC("Inconsistent. Unsatisfied constraints: ")(unsatisfied)("\n");
#ifdef BACKJUMPING
            set<VariableId> possibly_conflicted_vars = GetAllAffected(unsatisfied);
            TRACEC("Possible conflicted variables: ")(possibly_conflicted_vars)("\n");
#endif
            bool backjump = false;
            do
            {
                value_selectors.erase(*current_it);            
                TRACEC("Killed selector for ")(*current_it)("\n");
                
                if( current_it == plan.variables.begin() )
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
            if( current_it != plan.variables.end() ) // new variable
            {
                value_selectors[*current_it] = 
                    make_shared<ValueSelector>( plan, *this, knowledge, assignments, current_it );     
                TRACEC("Advanced to and made selector for ")(*current_it)("\n");
            }
            else // complete
            {
                TRACEC("Reporting solution\n");
                holder->ReportSolution( assignments );
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
    current_var( *current_it )
{
    ASSERT( current_it != solver_plan.variables.end() );
    ASSERT( assignments.count(current_var) == 0 );
    INDENT("V");
       
    Value start_val = knowledge->ordered_domain.front();
    
#ifdef DYNAMIC_START_VALUE    
    if( current_it!=solver_plan.variables.begin() )
    {
        list<VariableId>::const_iterator prev_it = current_it;
        --prev_it; 
        start_val = assignments.at(*prev_it);   
    }
#endif
    
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
    assignments.erase(current_var);
}


pair<Value, SimpleSolver::ConstraintSet> SimpleSolver::ValueSelector::SelectNextValue()
{
    INDENT("N");    
    TRACE("Finding value for variable ")(current_var)("\n");
    while( !value_queue.empty() )
    {       
        Value value = value_queue.front();
        assignments[current_var] = value;
        value_queue.pop_front();
        
        bool ok;
        Assignment hint;  
        ConstraintSet unsatisfied;     
        tie(ok, hint, unsatisfied) = solver.Test( assignments, solver_plan.affected_constraints.at(current_var) );        
        ASSERT( ok || !unsatisfied.empty() );

#ifdef TAKE_HINTS
        if( !ok && hint && current_var==(VariableId)(hint) ) // we got a hint, and for the current variable
        {
            TRACE("At ")(current_var)(", got hint ")(hint)(" - rewriting queue\n"); 
            value_queue.clear();
            value_queue.push_back( (Value)(hint) ); 
        }
#endif
       
        all_unsatisfied = UnionOf(all_unsatisfied, unsatisfied);
       
        if( ok )
        {
            TRACEC("Value is ")(value)("\n");
            return make_pair(value, all_unsatisfied);
        }
    }
    TRACEC("No (more) values found\n");
    ASSERT( !all_unsatisfied.empty() ); // Note: could fire if domain is empty
    return make_pair(Value(), all_unsatisfied);
}


tuple<bool, Assignment, SimpleSolver::ConstraintSet> SimpleSolver::Test( const Assignments &assigns, const ConstraintSet &to_test ) const 
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
    } 
    return make_tuple( unsatisfied.empty(), 
                       hints.empty() ? Assignment() : hints.front(),
                       unsatisfied );
}


void SimpleSolver::TraceProblem() const
{
    TRACE("SimpleSolver SS%d; %d constraints:", my_index, plan.constraints.size());
    for( shared_ptr<Constraint> c : plan.constraints )    
        c->TraceProblem();   
    TRACEC("%d variables:", plan.variables.size());        
    for( VariableId var : plan.variables )
        TRACEC(var)("\n");
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


set<VariableId> SimpleSolver::GetAllAffected( ConstraintSet constraints )
{
    set<VariableId> all_vars;
    for( shared_ptr<Constraint> c : constraints )
        all_vars = UnionOf(all_vars, c->GetFreeVariables());
    return all_vars;            
}


int SimpleSolver::next_index = 0;
