#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "constraint.hpp"

#include "node/specialise_oostd.hpp"
#include "common/common.hpp"

#include <functional>

namespace SR
{
class XTreeDatabase;
}

namespace CSP
{     
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
class Solver : public Traceable,
               public SerialNumber
{
public:

    /**
     * Function supplied to Solver objects for discovered solution reportage.
     */    
    typedef function<void(const Solution &solution)> SolutionReportFunction;
    
    /**
     * Function supplied to Solver objects for rejected partial assignment reportage.
     */    
    typedef function<void(const Assignments &assigns)> RejectionReportFunction;
    
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
     * Prepare to solve.
     * 
     * @param forces [in] partial assignment with values for all forced variables.
     * 
     * @param x_tree_db [in] database of information about the current values.
     */
    virtual void Start( const Assignments &forces,
                        const SR::XTreeDatabase *x_tree_db ) = 0;
                      
    /**
     * Run the solver to exhaustion (i.e. it will discover all the
     * solutions). Solutions will be reported using the supplied function. If that
     * function wants to abort the solve, it should throw an exception.    
     * 
     * @param solution_report_function [inout] solutions reported by calling this (required).
     * 
     * @param rejection_report_function [inout] rejections reported by calling this (optional).
     */
    virtual void Run( const SolutionReportFunction &solution_report_function,
                      const RejectionReportFunction &rejection_report_function = nullptr ) = 0;

    string GetTrace() const;

    /**
     * Write info about the problem to log via TRACE
     */
    virtual void Dump() const = 0;

    /**
     * Check for coherence in the problem we've been asked to plan for
     */
    virtual void CheckPlan() const = 0;
};

};

#endif
