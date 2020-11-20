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


void SolverHolder::Start( const set<Value> &initial_domain, 
                          const Assignments &forces )
{
    solutions_queue.clear();
    solver->Run( this, initial_domain, forces);
}


void SolverHolder::ReportSolution( const Solution &solution )
{
    solutions_queue.push_back( solution );
}


bool SolverHolder::GetNextSolution( Solution *solution )
{
    if( solutions_queue.empty() )
        return false;
    if( solution )
        *solution = solutions_queue.front();
    solutions_queue.pop_front();
    return true;
}