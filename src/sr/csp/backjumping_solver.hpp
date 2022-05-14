#ifndef BACKJUMPING_SOLVER_HPP
#define BACKJUMPING_SOLVER_HPP

#include "reference_solver.hpp"
#include <list>
#include <vector>

namespace CSP
{ 

class BackjumpingSolver : public ReferenceSolver
{
public:
    BackjumpingSolver( const list< shared_ptr<Constraint> > &constraints, 
                       const vector<VariableId> &free_variables, 
                       const vector<VariableId> &forced_variables );
                       
private:    
    void Solve() override;
    void AssignSuccessful() override;    
    bool AssignUnsuccessful() override;    

    int conflicted_count;       
};

}

#endif
