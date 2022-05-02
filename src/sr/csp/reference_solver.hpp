#ifndef REFERENCE_SOLVER_HPP
#define REFERENCE_SOLVER_HPP

#include "constraint.hpp"
#include "solver.hpp"

#include "node/specialise_oostd.hpp"
#include "common/common.hpp"
#include "../the_knowledge.hpp"
#include "value_selector.hpp"

#include <chrono>
#include <functional>

namespace CSP
{ 
    
class Agent;
class SolverHolder;

/** A simple back-tracking solver
 */
class ReferenceSolver : public Solver
{
public:
    typedef tuple<bool, ConstraintSet> CCRV;
    typedef pair<Value, ConstraintSet> SelectNextValueRV;

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
    ReferenceSolver( const list< shared_ptr<Constraint> > &constraints, 
                     const vector<VariableId> &free_variables, 
                     const vector<VariableId> &forced_variables );
    ~ReferenceSolver();

    virtual void Start( const Assignments &forces,
                        const SR::TheKnowledge *knowledge );
    virtual void Run( const SolutionReportFunction &solution_report_function,
                      const RejectionReportFunction &rejection_report_function );

protected:
    const struct Plan : public virtual Traceable
    {
        Plan( ReferenceSolver *algo,
              const list< shared_ptr<Constraint> > &constraints, 
              const vector<VariableId> &free_variables, 
              const vector<VariableId> &forced_variables );
        void DeduceVariables();
        string GetTrace() const; // used for debug
    
        ReferenceSolver * const algo;
        const list< shared_ptr<Constraint> > constraints;
        const vector<VariableId> free_variables;
        const vector<VariableId> forced_variables;
        
        map<VariableId, int> free_variables_to_indices;
        ConstraintSet constraint_set;
        ConstraintSet fully_forced_constraint_set;
        map< shared_ptr<Constraint>, set<int> > free_var_indices_for_constraint;

        vector<ConstraintSet> affected_constraints; // does not depend on var ordering
        vector<ConstraintSet> completed_constraints; // depends on var ordering
    } plan;

    virtual void Solve();
    virtual void AssignSuccessful();    
    virtual bool AssignUnsuccessful();    
    SelectNextValueRV TryFindNextConsistentValue( int my_var_index );
    CCRV ConsistencyCheck( const Assignments &assigns,
                           const ConstraintSet &to_test ) const;
    void ShowBestAssignment();
    void TimedOperations();
    void CheckPlan() const;
    set<int> GetAffectedVariableIndices( ConstraintSet constraints );

    void Dump() const;

    // Structural
    SolutionReportFunction solution_report_function;
    RejectionReportFunction rejection_report_function;
    
    // Used during solve - depends on pattern and x
    const SR::TheKnowledge *knowledge;
    Assignments forced_assignments;
        
    int current_var_index;
    Assignments assignments;
    map< int, shared_ptr<ValueSelector> > value_selectors;
    map< int, int > success_count;
    
    // Timed reports
    chrono::time_point<chrono::steady_clock> last_report;
    
public:
    static void DumpGSV();
};

};

#endif
