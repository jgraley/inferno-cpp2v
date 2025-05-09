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
#ifdef COROUTINE_HOLDER
    ReapSource();
#endif
}


void SolverHolder::Start( const Assignments &forces,
                          const SR::XTreeDatabase *x_tree_db )
{
    solver->Start( forces, x_tree_db );    

#ifdef COROUTINE_HOLDER
    ReapSource();
    if( source )
        delete source; // will unwind any stack frames using an exception
    
    auto lambda = [&](Coroutine::push_type& sink_)
    {        
		auto solution_report_lambda = [&](const Solution &solution)
		{ 
			ReportSolution(solution); 
		};

        sink = &sink_;
        (*sink)( {} ); // Yield so we don't do "real work" until GetNextSolution(), rule #393
        
        try
        {            
            solver->Run( solution_report_lambda, Solver::RejectionReportFunction() );
        }
        catch(const exception &e)
        {
            // Don't log the exception because we'll re-throw
            solver_exception = current_exception();
            (*sink)( {} );
        }
        sink = nullptr;
    };
    
    source = new Coroutine::pull_type(lambda);    
    MaybeRethrow();
#else
	auto solution_report_lambda = [&](const Solution &solution)
	{ 
		ReportSolution(solution); 
	};

    solutions_queue.clear();
    solver->Run( solution_report_lambda, Solver::RejectionReportFunction() );
#endif
}


bool SolverHolder::GetNextSolution( Solution *solution )
{
#ifdef COROUTINE_HOLDER
    ASSERT( source );
    // Yield to solver first so that the portion of the solver's
    // work done during this call to GetNextSolution() corresponds to 
    // the solution (or exception, or completion) we return from this 
    // call. We prefer not to let solver run ahead. This is so the logs 
    // etc are easier to understand. We're not in the business of 
    // seeking concurrancy. See #393
    (*source)();
    ReapSource();
    if( !source )
        return false;
    *solution = source->get();
    MaybeRethrow();
    return true;
#else
    if( solutions_queue.empty() )
        return false;
    if( solution )
        *solution = solutions_queue.front();
    solutions_queue.pop_front();
    return true;
#endif
}


void SolverHolder::ReportSolution( const Solution &solution )
{
#ifdef COROUTINE_HOLDER
    ASSERT( sink );
    (*sink)( solution );
#else
    ASSERT( solutions_queue.size() < 1000000 );
    solutions_queue.push_back( solution );   
#endif
}


#ifdef COROUTINE_HOLDER
void SolverHolder::ReapSource()
{
    if( source && !*source )
    {
        delete source;
        source = nullptr;
    }
}


void SolverHolder::MaybeRethrow()
{
    if( solver_exception )
    {
        exception_ptr t = solver_exception;
        solver_exception = nullptr;
        rethrow_exception(t);
    }
}
#endif


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
