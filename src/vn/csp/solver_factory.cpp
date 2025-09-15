#include "solver_factory.hpp"

#include "solver.hpp"
#include "reference_solver.hpp"
#include "backjumping_solver.hpp"
#include "solver_test.hpp"
#include "common/read_args.hpp"

using namespace CSP;

//#define USE_REF_SOLVER_ONLY

shared_ptr<CSP::Solver> CSP::CreateSolverAndHolder( const list< shared_ptr<Constraint> > &constraints, 
                                                    const vector<VariableId> &free_variables, 
                                                    const set<VariableId> &domain_forced_variables, 
                                                    const set<VariableId> &arbitrary_forced_variables )
{
#ifdef USE_REF_SOLVER_ONLY
    auto salg = make_shared<CSP::ReferenceSolver>( constraints, 
                                                   free_variables, 
                                                   domain_forced_variables, 
                                                   arbitrary_forced_variables );
#else                                                   
    auto salg = make_shared<CSP::BackjumpingSolver>( constraints, 
                                                     free_variables, 
                                                     domain_forced_variables, 
                                                     arbitrary_forced_variables );
#endif

    if( ReadArgs::test_csp )
    {
        auto refalg = make_shared<CSP::ReferenceSolver>( constraints, 
                                                         free_variables, 
                                                         domain_forced_variables, 
                                                         arbitrary_forced_variables );

        auto testalg = make_shared<CSP::SolverTest>( refalg, salg );

        return testalg;
    }
    else
    {
        return salg;
    }
}                                                       
