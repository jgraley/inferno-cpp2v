#include "solver_test.hpp"

using namespace CSP;

SolverTest::SolverTest( shared_ptr<Solver> reference_solver_,
                        shared_ptr<Solver> solver_under_test_ ) :
    reference_solver( reference_solver_ ),
    solver_under_test( solver_under_test_ )
{
}

void SolverTest::Start( const Assignments &forces,
                        const SR::XTreeDatabase *x_tree_db )
{
    reference_solver->Start(forces, x_tree_db);
    solver_under_test->Start(forces, x_tree_db);
}


void SolverTest::Stop()
{
	reference_solver->Stop();
	solver_under_test->Stop();
}


void SolverTest::Run( const SolutionReportFunction &solution_report_function,
                      const RejectionReportFunction &rejection_report_function )
{
    // Get a set of solutions from the reference solver
    TRACE("############ RUNNING REFERENCE SOLVER #############\n");
    set<Solution> reference_solutions, under_test_solutions;
    auto reference_srl = [&](const Solution &solution)
    {         
        reference_solutions.insert( solution );
    };
    reference_solver->Run( reference_srl, RejectionReportFunction() );
    
    // Run solver-under-test and check both solutions and rejections
    TRACE("############ RUNNING SOLVER UNDER TEST #############\n");
    auto under_test_srl = [&](const Solution &solution)
    { 
        ASSERT( reference_solutions.count( solution ) > 0 );
        under_test_solutions.insert( solution );
    };
    auto under_test_rrl = [&](const Assignments &assigns)
    { 
        for( const Solution &s : reference_solutions )
        {
            ASSERT( !IsIncludes( s, assigns ) )
                  ("Reference assignment was rejected by solver under test");
        }
    };
    solver_under_test->Run( under_test_srl, under_test_rrl );
    
    if( under_test_solutions.size() != reference_solutions.size() )
    {
        TRACE("############ DUMP REFERENCE SOLVER #############\n");
        reference_solver->Dump();
    }
    ASSERT( under_test_solutions.size() == reference_solutions.size() )
          ("\n############ UNDER TEST SOLUTIONS #############\n")(under_test_solutions)
          ("\n############ REFERENCE SOLUTIONS #############\n")(reference_solutions);
    
    // Wait till we've passed before reporting solutions, because it generates logging
    for( const Solution &s : reference_solutions )
    {
        solution_report_function( s );
    }
}

          
void SolverTest::Dump() const
{
    reference_solver->Dump();
    solver_under_test->Dump();
}


void SolverTest::CheckPlan() const
{
    reference_solver->Dump();
    solver_under_test->Dump();
}
