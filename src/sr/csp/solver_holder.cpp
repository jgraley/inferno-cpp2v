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


void SolverHolder::Start( const set<Value> &initial_domain_ )
{
    matches.clear();
    solver->Run( this, initial_domain_ );
}


void SolverHolder::ReportSolution( const map< shared_ptr<Constraint>, list< Value > > &values )
{
    matches.push_back( values );
}


bool SolverHolder::GetNextSolution( map< shared_ptr<Constraint>, list< Value > > *values )
{
    if( matches.empty() )
        return false;
    if( values )
        *values = matches.front();
    matches.pop_front();
    return true;
}