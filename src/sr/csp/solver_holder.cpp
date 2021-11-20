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
                          const SR::TheKnowledge *knowledge )
{
#ifdef COROUTINE_HOLDER
    ReapSource();
    if( source )
        delete source; // will unwind any stack frames using an exception
    auto lambda = [&](Coroutine::push_type& sink_)
    {
        sink = &sink_;
        try
        {
            solver->Run( this, forces, knowledge);
        }
        catch(const exception& e)
        {
            solver_exception = current_exception();
            (*sink)( {} );
        }
        sink = nullptr;
    };
    source = new Coroutine::pull_type(lambda);    
    MaybeRethrow();
#else
    solutions_queue.clear();
    solver->Run( this, forces, knowledge);
#endif
}


bool SolverHolder::GetNextSolution( Solution *solution )
{
#ifdef COROUTINE_HOLDER
    ASSERT( source );
    ReapSource();
    if( !source )
        return false;
    *solution = source->get();
    (*source)();
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
