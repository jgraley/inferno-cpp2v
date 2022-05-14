#include "backjumping_solver.hpp"

using namespace CSP;


BackjumpingSolver::BackjumpingSolver( const list< shared_ptr<Constraint> > &constraints, 
                                      const vector<VariableId> &free_variables, 
                                      const vector<VariableId> &forced_variables ) :
    ReferenceSolver( constraints, free_variables, forced_variables )
{
}


void BackjumpingSolver::Solve()
{
    conflicted_count = 0;
    ReferenceSolver::Solve();
}


void BackjumpingSolver::AssignSuccessful()
{
    conflicted_count = 0;
    ReferenceSolver::AssignSuccessful();
}


bool BackjumpingSolver::AssignUnsuccessful()
{
    const ConstraintSet &suspect = plan.affected_constraints.at(current_var_index);
    TRACEC("Inconsistent. Possible conflicted constraints: ")(suspect)("\n");
    //TRACEC("All affected: ")(plan.affected_constraints.at(current_var_index))("\n");

    set<int> possibly_conflicted_vars = GetAffectedVariableIndices(suspect);
    TRACEC("Possible conflicted variables: ")(possibly_conflicted_vars)("\n");
    bool can_backjump = (conflicted_count==0 && success_count.at(current_var_index)==0);
    bool backjump = false;
    do
    {
        bool cease = ReferenceSolver::AssignUnsuccessful();
        if( cease )
            return true;
        
        backjump = ( possibly_conflicted_vars.count(current_var_index) == 0 &&
                     can_backjump );        
    } while( backjump ); // backjump into possibly_conflicted_vars
        
    conflicted_count++;
    return false;
}