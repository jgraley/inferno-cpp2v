#include "solver_factory.hpp"

#include "solver.hpp"
#include "reference_solver.hpp"
#include "backjumping_solver.hpp"
#include "solver_test.hpp"
#include "common/read_args.hpp"

using namespace CSP;

shared_ptr<CSP::SolverHolder> CSP::CreateSolverAndHolder( const list< shared_ptr<Constraint> > &constraints, 
                                                          const vector<VariableId> &free_variables, 
                                                          const vector<VariableId> &forced_variables )
{
    auto salg = make_shared<CSP::BackjumpingSolver>( constraints, 
                                                     free_variables, 
                                                     forced_variables );

    if( ReadArgs::csp_test )
    {
        auto refalg = make_shared<CSP::ReferenceSolver>( constraints, 
                                                         free_variables, 
                                                         forced_variables );

        auto testalg = make_shared<CSP::SolverTest>( refalg, salg );

        return make_shared<CSP::SolverHolder>( testalg );
    }
    else
    {
        return make_shared<CSP::SolverHolder>( salg );
    }
}                                                       