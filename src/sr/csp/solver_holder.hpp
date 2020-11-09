#ifndef SOLVER_HOLDER_HPP
#define SOLVER_HOLDER_HPP

#include "constraint.hpp"
#include "solver.hpp"

#include "node/specialise_oostd.hpp"
#include "common/common.hpp"

namespace CSP
{ 
    
class Agent;
class Solver;
    
/** A simple back-tracking solver
 */
class SolverHolder : private Solver::ReportageObserver
{
public:
    /**
     * Create a holder for a solver. The holder is bound to the solver, and the solver should
     * probably be considered bound to the holder.
     * 
     * @param solver [inout] the solver to bind to
     */
    SolverHolder( shared_ptr<Solver> solver );

    /**
     * Start the process of solving a given example of a problem. The solver 
     * is expected to already know the constraints, and variables (which can
     * be dediced from the constraint) and so all we require to get started
     * is the domain, which is common to all the variables.
     * 
     * @param initial_domain [in] the domain for all the variables, could get expanded depending on the solver algorithm.
     */
    void Start( const set<Value> &initial_domain, 
                const Solver::Assignments &forces );

    /**
     * Try to extract a single solution from the solver. 
     * 
     * @param values [out] if non-null and solution exists: values that solve the CSP, organsied into a list for each constraint. Each list is ordered as per `Constraint::GetFreeVariables()`.
     * 
     * @param side_info [out] if non-null and solution exists: side-information as required by the `AndRuleEngine` in order to make use of the solution.
     *
     * @retval true A solution was found, and non-null arguments are filled in
     * @retval false No solutions are left, and the pointer arguments are unused
     */
    bool GetNextSolution( Solver::Solution *solution = nullptr );

 
private:
    void ReportSolution( const Solver::Solution &solution );

    const shared_ptr<Solver> solver;

    list<Solver::Solution> solutions_queue;    
};

};

#endif
