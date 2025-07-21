#ifndef SOLVER_HOLDER_HPP
#define SOLVER_HOLDER_HPP

#include "constraint.hpp"
#include "solver.hpp"

#include "node/specialise_oostd.hpp"
#include "common/common.hpp"

#include <functional>

#define STACKED_CSP

namespace SR
{
class Agent;
class XTreeDatabase;
}

namespace CSP
{ 
class Solver;

typedef function<void(Solution)> SolutionHandler;

/** A simple back-tracking solver
 */
class SolverHolder : public Traceable
{
public:
    /**
     * Create a holder for a solver. The holder is bound to the solver, and the solver should
     * probably be considered bound to the holder.
     * 
     * @param solver [inout] the solver to bind to
     */
    SolverHolder( shared_ptr<Solver> solver );
    ~SolverHolder();

    /**
     * Start the process of solving a given example of a problem. The solver 
     * is expected to already know the constraints, and variables (which can
     * be dediced from the constraint) and so all we require to get started
     * is the domain, which is common to all the variables.
     * 
     * @param initial_domain [in] the domain for all the variables, could get expanded depending on the solver algorithm.
     */
    void Start( const Assignments &forces,
                const SR::XTreeDatabase *x_tree_db,
                SolutionHandler solution_handler );

    /**
     * Try to extract a single solution from the solver. 
     * 
     * @param solution [out] if non-null and solution exists: A fullly populated assignments map
     *
     * @retval true A solution was found, and non-null arguments are filled in
     * @retval false No solutions are left, and the pointer arguments are unused
     */
    bool GetNextSolution( Solution *solution = nullptr );
 
    string GetTrace() const;

    /**
     * Write info about the problem to log via TRACE
     */
    void Dump() const;

    /**
     * Check for coherence in the problem we've been asked to plan for
     */
    void CheckPlan() const;
    
private:
    void ReportSolution( const Solution &solution );    
    const shared_ptr<Solver> solver;

    bool enable_coroutine;
    list<Solution> solutions_queue;    
};

};

#endif
