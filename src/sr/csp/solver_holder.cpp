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


void SolverHolder::Start( const Assignments &forces,
                          const SR::XTreeDatabase *x_tree_db )
{
    solver->Start( forces, x_tree_db );    

	auto solution_report_lambda = [&](const Solution &solution)
	{ 
		ReportSolution(solution); 
	};

    solutions_queue.clear();
    solver->Run( solution_report_lambda, Solver::RejectionReportFunction() );
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


void SolverHolder::ReportSolution( const Solution &solution )
{
    ASSERT( solutions_queue.size() < 1000000 );
    solutions_queue.push_back( solution );   
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
