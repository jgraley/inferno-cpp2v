#ifndef SOLVER_FACTORY_HPP
#define SOLVER_FACTORY_HPP

#include "solver_holder.hpp"
#include "constraint.hpp"

namespace CSP
{     
shared_ptr<CSP::SolverHolder> CreateSolverAndHolder( const list< shared_ptr<Constraint> > &constraints, 
                                                     const vector<VariableId> &free_variables, 
                                                     const vector<VariableId> &domain_forced_variables, 
                                                     const vector<VariableId> &arbitrary_forced_variables );
};

#endif
