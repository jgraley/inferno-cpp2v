#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "constraint.hpp"

#include "node/node.hpp"
#include "common/common.hpp"

namespace CSP
{ 
    
class SolverHolderReportInterface;
    
/** Interface class for constraint problem solvers. 
 * 
 * This class is unstated - i.e.
 * it insists on doing an entire problem-solve in a single method call (called
 * `Run()`). It's not sufficient to merely throw an exception on a hit to 
 * simplify unwind, since it's likely to produce multiple solutions and we 
 * may not be able to use the first (eg a sub-engine mismatches). So we 
 * provide an observer interface and leave it to that class's implementation 
 * to "just deal with it". This class has no idea how that will happen
 * but hopes it will be both fun and safe.
 */
class Solver
{
public:
    /**
     * Interface presented to Solver objects for discovered solution reportage.
     */    
    class ReportageObserver
    {
    public:
        /**
         * Report that a single solution has been found. 
         * 
         * @param values [in] values that solve the CSP, organsied into a list for each constraint. Each list is ordered as per `Constraint::GetFreeVariables()`.
         * 
         * @param side_info [in] side-information as required by the `AndRuleEngine` in order to make use of the solution.
         */
        virtual void ReportSolution( const map< shared_ptr<Constraint>, list< Value > > &values, 
                                     shared_ptr<SideInfo> side_info ) = 0;
    }; 
    
    /** Create a solver object.
     * 
     * It is anticipated that implementations will accept a 
     * `const list< shared_ptr<Constraint> > &constraints` and possibly
     * more depending on solver algorithm. This will be all the 
     * constraints to satisfy.
     */
    Solver();
    
    /**
     * Desroy an instance of a solver on the basis that it has outlived
     * it's usefulness and is now less valuable than the memory it consumes.
     */
    virtual ~Solver();
    
    /**
     * Run the solver to exhaustion (i.e. it will discover all the
     * solutions) given an initial domain. Solutions will be reported
     * back to the supplied holder class.
     * 
     * @param holder [inout] solutions reported to this object via `ReportSolution()`
     * 
     * @param initial_domain [in] the domain for all the variables, could get expanded depending on the solver algorithm.
     */
    virtual void Run( ReportageObserver *holder, 
                      const set<Value> &initial_domain ) = 0;
};

};

#endif
