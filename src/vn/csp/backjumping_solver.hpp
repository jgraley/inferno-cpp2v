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
    using ReferenceSolver::ReferenceSolver;
                       
private:    
    void Solve() override;
    void AssignSuccessful() override;    
    bool AssignUnsuccessful() override;    

    int conflicted_count;       
};

}

#endif
