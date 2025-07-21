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
                              const SR::XTreeDatabase *x_tree_db,
                              SolutionHandler solution_handler )                            
{
    solver->Start( forces, x_tree_db );    

    solutions_queue.clear();

#ifdef STACKED_CSP
    solver->Run( solution_handler, Solver::RejectionReportFunction() );
#else    
    (void)solution_handler;
	SolutionHandler solution_report_lambda = [&](Solution solution)
	{ 
		ReportSolution(solution); 
	};

    solver->Run( solution_report_lambda, Solver::RejectionReportFunction() );
#endif    
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
