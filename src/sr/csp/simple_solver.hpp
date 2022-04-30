#ifndef SIMPLE_SOLVER_HPP
#define SIMPLE_SOLVER_HPP

#include "constraint.hpp"
#include "solver.hpp"

#include "node/specialise_oostd.hpp"
#include "common/common.hpp"
#include "../the_knowledge.hpp"
#include "value_selector.hpp"

#include <chrono>
#include <functional>

//#define BACKJUMPING

namespace CSP
{ 
    
class Agent;
class SolverHolder;

/** A simple back-tracking solver
 */
class SimpleSolver : public Solver
{
public:
#ifdef BACKJUMPING
    typedef pair<Value, ConstraintSet> SelectNextValueRV;
    typedef tuple<bool, ConstraintSet> CCRV;
#else
    typedef Value SelectNextValueRV;
    typedef tuple<bool> CCRV;
#endif

    /**
     * Create a simple backtracking CSP solver. We require constraints at construct time and 
     * can only solve the problme implied by them. Optionally, for this solver class only, 
     * the varaibles may be supplied, in the order in which we should process them. This permits
     * our backtracking behaviour to be forced to match that of Conjecture etc.
     * 
     * @param [input] constraints the list of constraints that the solver will try to satisfy
     * 
     * @param [input] if non-null, the variables to use. Must be the same set that we would deduce from querying the constraints, but in any order.
     */
    SimpleSolver( const list< shared_ptr<Constraint> > &constraints, 
                  const list<VariableId> &free_variables, 
                  const list<VariableId> &forced_variables );

    virtual void Start( const Assignments &forces,
                        const SR::TheKnowledge *knowledge );
    virtual void Run( const SolutionReportFunction &solution_report_function,
                      const RejectionReportFunction &rejection_report_function );

private:
    const struct Plan : public virtual Traceable
    {
        Plan( SimpleSolver *algo,
              const list< shared_ptr<Constraint> > &constraints, 
              const list<VariableId> &free_variables, 
              const list<VariableId> &forced_variables );
        void DeduceVariables();
        string GetTrace() const; // used for debug
    
        SimpleSolver * const algo;
        const list< shared_ptr<Constraint> > constraints;
        const list<VariableId> free_variables;
        const list<VariableId> forced_variables;
        
        ConstraintSet constraint_set;
        ConstraintSet fully_forced_constraint_set;
        map< shared_ptr<Constraint>, set<VariableId> > free_vars_for_constraint;

        map<VariableId, ConstraintSet> affected_constraints; // does not depend on var ordering
        map<VariableId, ConstraintSet> completed_constraints; // depends on var ordering
    } plan;

    void Solve( list<VariableId>::const_iterator current_var_it );
    SelectNextValueRV TryFindNextConsistentValue( VariableId my_var );

    CCRV ConsistencyCheck( const Assignments &assigns,
                           const ConstraintSet &to_test ) const;
    void ShowBestAssignment();
    void TimedOperations();
    void CheckPlan() const;
    set<VariableId> GetAllAffected( ConstraintSet constraints );

    void Dump() const;

    // Structural
    SolutionReportFunction solution_report_function;
    RejectionReportFunction rejection_report_function;
    
    // Used during solve - depends on pattern and x
    const SR::TheKnowledge *knowledge;
    Assignments forced_assignments;
    Assignments assignments;
    map< VariableId, shared_ptr<ValueSelector> > value_selectors;
    
#ifdef BACKJUMPING
    int conflicted_count;
#endif    
    
    // Timed reports
    chrono::time_point<chrono::steady_clock> last_report;
public:
    static void DumpGSV();
};

};

#endif
