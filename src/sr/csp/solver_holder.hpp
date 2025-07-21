#ifndef SOLVER_HOLDER_HPP
#define SOLVER_HOLDER_HPP

#include "constraint.hpp"
#include "solver.hpp"

#include "node/specialise_oostd.hpp"
#include "common/common.hpp"

#include <functional>

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
    void RunSolver( const Assignments &forces,
                    const SR::XTreeDatabase *x_tree_db,
                    SolutionHandler solution_handler );
 
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
    const shared_ptr<Solver> solver;
};

};

#endif
