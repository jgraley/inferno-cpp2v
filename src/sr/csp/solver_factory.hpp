#ifndef SOLVER_FACTORY_HPP
#define SOLVER_FACTORY_HPP

#include "solver_holder.hpp"
#include "constraint.hpp"

namespace CSP
{     
shared_ptr<CSP::SolverHolder> CreateSolverAndHolder( const list< shared_ptr<Constraint> > &constraints, 
                                                     const list<VariableId> &free_variables, 
                                                     const list<VariableId> &forced_variables );
};

#endif
