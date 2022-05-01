#ifndef SOLVER_TEST_HPP
#define SOLVER_TEST_HPP

#include "solver.hpp"

namespace CSP
{ 

class SolverTest : public Solver
{
public:
    SolverTest( const list< shared_ptr<Constraint> > &constraints, 
                const list<VariableId> &free_variables, 
                const list<VariableId> &forced_variables );    
};

}

#endif
