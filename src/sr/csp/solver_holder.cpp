#include "solver_holder.hpp"
#include "solver.hpp"
#include "query.hpp"
#include "agents/agent.hpp"

#include <algorithm>



using namespace CSP;

SolverHolder::SolverHolder( shared_ptr<Solver> solver_ ) :
    solver( solver_ )
{
}


SolverHolder::~SolverHolder()
{
}


void SolverHolder::RunSolver( const Assignments &forces,
                              const SR::XTreeDatabase *x_tree_db,
                              SolutionHandler solution_handler )                            
{
    solver->Start( forces, x_tree_db );    

    solver->Run( solution_handler, Solver::RejectionReportFunction() );  
}


string SolverHolder::GetTrace() const
{
    return GetName() + "(" + (solver?solver->GetTrace():"") + ")";
}


void SolverHolder::Dump() const
{
    ASSERT( solver );
    solver->Dump();
}


void SolverHolder::CheckPlan() const
{
    ASSERT( solver );
    solver->CheckPlan();
}
