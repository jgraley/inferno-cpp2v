#include "solver_holder.hpp"
#include "solver.hpp"
#include "query.hpp"
#include "agent.hpp"

#include <algorithm>

using namespace CSP;


SolverHolder::SolverHolder( shared_ptr<Solver> solver_ ) :
    solver( solver_ )
{
}


void SolverHolder::Start( const set<Value::Id> &initial_domain_ )
{
    matches.clear();
    solver->Run( this, initial_domain_ );
}


void SolverHolder::ReportSolution( const map< shared_ptr<Constraint>, list< Value::Id > > &values, 
                                   shared_ptr<SideInfo> side_info )
{
    matches.push_back( make_pair(values, side_info) );
}


bool SolverHolder::GetNextSolution( map< shared_ptr<Constraint>, list< Value::Id > > *values, 
                                    SideInfo *side_info )
{
    if( matches.empty() )
        return false;
    if( values )
        *values = matches.front().first;
    if( side_info )
        *side_info = *matches.front().second;
    matches.pop_front();
    return true;
}