#include "solver_factory.hpp"

#include "solver.hpp"
#include "reference_solver.hpp"
#include "backjumping_solver.hpp"

using namespace CSP;

shared_ptr<CSP::SolverHolder> CSP::CreateSolverAndHolder( const list< shared_ptr<Constraint> > &constraints, 
                                                          const list<VariableId> &free_variables, 
                                                          const list<VariableId> &forced_variables )
{
#if 0
    auto salg = make_shared<CSP::BackjumpingSolver>( constraints, 
                                                     free_variables, 
                                                     forced_variables );
#else
    auto salg = make_shared<CSP::ReferenceSolver>( constraints, 
                                                   free_variables, 
                                                   forced_variables );
#endif

    return make_shared<CSP::SolverHolder>(salg);
}                                                       
