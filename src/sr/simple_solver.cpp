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
    for( VariableId var : variables )
    {
        assignments[var] = NullValue; // means "unassigned"
    }
    
#ifdef TRACK_BEST_ASSIGNMENT    
    best_assignments.clear();
    best_num_assignments = 0;
#endif    
    
    TryVariable( variables.begin() );    

#ifdef TRACK_BEST_ASSIGNMENT    
    TRACE("Best assignment assigned %d of %d/%d variables:\n", best_num_assignments, assignments.size(), best_assignments.size());
    ASSERT( assignments.size() == best_assignments.size() );
    for( VariableId var : variables )
    {
        if( best_assignments.at(var) != NullValue )
        {
            TRACE("Variable ")(*var)(" assigned ")(*best_assignments.at(var))("\n");
        }
        else 
        {
            TRACE("Variable ")(*var)(" could not be matched after trying:\n");
            for( Value val : initial_domain )
                TRACE(*val)("\n");                
            break;
        }
    }
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
        
        bool ok = Test( assignments, side_info.get() ); // Ask for side info if assignment is complete
        
        if( !ok )
        {
            assignments[*current] = NullValue; // put failed variable back to NULL
            continue; // backtrack
        }
            
#ifdef TRACK_BEST_ASSIGNMENT       
        int num=0;
        for( auto p : assignments )
            num += ( p.second != NullValue );
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

bool SimpleSolver::Test( map<VariableId, Value> &assigns, 
                         SideInfo *side_info )
{
    bool ok = true; // AND-rule
    for( shared_ptr<Constraint> c : constraints )
    {
        list<Value> vals = GetConstraintValues( c, assignments );

        if( vals.size() < c->GetFreeDegree() )
            continue; // There were unassigned variables, don't bother testing this constraint
            
        ok = c->Test(vals, side_info); 
        
        if( !ok )
            break;
    }        
    return ok;
}


list<Value> SimpleSolver::GetConstraintValues( shared_ptr<Constraint> c, const Assignments &a )
{
    list<VariableId> vars = c->GetFreeVariables();
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


void SimpleSolver::ReportSolution( const Assignments &assignments, 
                                   shared_ptr<SideInfo> side_info )
{
    map< shared_ptr<Constraint>, list< Value > > vals;
    vals.clear();
    for( shared_ptr<Constraint> c : constraints )
    {
        vals[c] = GetConstraintValues(c, assignments);
        ASSERT( vals.at(c).size() == c->GetFreeDegree() ); // Assignments should be complete
    }
    
    holder->ReportSolution( vals, side_info );
}                     

