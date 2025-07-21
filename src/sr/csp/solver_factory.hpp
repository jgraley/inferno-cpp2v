#ifndef SOLVER_FACTORY_HPP
#define SOLVER_FACTORY_HPP

#include "constraint.hpp"
#include "solver.hpp"

namespace CSP
{     
shared_ptr<CSP::Solver> CreateSolverAndHolder( const list< shared_ptr<Constraint> > &constraints, 
                                               const vector<VariableId> &free_variables, 
                                               const set<VariableId> &domain_forced_variables, 
                                               const set<VariableId> &arbitrary_forced_variables );
};

#endif
