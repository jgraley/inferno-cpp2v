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


void SolverHolder::Start( const unordered_set<Value> &initial_domain, 
                          const Assignments &forces,
                          const SR::TheKnowledge *knowledge )
{
#ifdef COROUTINE_HOLDER
    ReapSource();
    if( source )
        delete source; // will unwind any stack frames using an exception
    auto lambda = [&](Coroutine::push_type& sink_)
    {
        sink = &sink_;
        solver->Run( this, initial_domain, forces, knowledge);
        sink = nullptr;
    };
    source = new Coroutine::pull_type(lambda);        
#else
    solutions_queue.clear();
    solver->Run( this, initial_domain, forces, knowledge);
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


void SolverHolder::ReapSource()
{
    if( source && !*source )
    {
        delete source;
        source = nullptr;
    }
}

