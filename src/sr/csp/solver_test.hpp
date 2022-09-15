#ifndef SOLVER_TEST_HPP
#define SOLVER_TEST_HPP

#include "solver.hpp"

namespace CSP
{ 

class SolverTest : public Solver
{
public:
    SolverTest( shared_ptr<Solver> reference_solver,
                shared_ptr<Solver> solver_under_test );    

    void Start( const Assignments &forces,
                const SR::XTreeDatabase *x_tree_db ) override;
    void Run( const SolutionReportFunction &solution_report_function,
              const RejectionReportFunction &rejection_report_function ) override;
    void Dump() const override;
    void CheckPlan() const override;

private:
    shared_ptr<Solver> reference_solver;
    shared_ptr<Solver> solver_under_test;    
};

}

#endif
