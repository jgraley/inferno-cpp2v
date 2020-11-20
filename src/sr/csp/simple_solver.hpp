#ifndef SIMPLE_SOLVER_HPP
#define SIMPLE_SOLVER_HPP

#include "constraint.hpp"
#include "solver.hpp"

#include "node/specialise_oostd.hpp"
#include "common/common.hpp"

#include <chrono>

namespace CSP
{ 
    
class Agent;
class SolverHolder;
    
/** A simple back-tracking solver
 */
class SimpleSolver : public Solver
{
public:
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
                  const list<VariableId> *variables = nullptr );

    void Run( ReportageObserver *holder, 
              const unordered_set<Value> &initial_domain,
              const Assignments &forces );

private:
    const struct Plan
    {
        Plan( const list< shared_ptr<Constraint> > &constraints, 
              const list<VariableId> *variables = nullptr );
        void DeduceVariables( const list<VariableId> *variables );

        list< shared_ptr<Constraint> > constraints;
        list<VariableId> variables;
    } plan;

    bool TryVariable( list<VariableId>::const_iterator current );
    bool Test( const Assignments &assigns );
    list<Value> GetValuesForConstraint( shared_ptr<Constraint> c, const Assignments &a );    
    void TraceProblem() const;
    static void CheckLocalMatch( const Assignments &assignments, VariableId variable );
    void ShowBestAssignment();
    void TimedOperations();

    ReportageObserver *holder;
        
    // Used during solve - depends on pattern and x
    unordered_set<Value> initial_domain;    
    Assignments assignments;
    list<int> try_counts; // just for tracing
    
    // Only needed for debug output
    Assignments best_assignments;
    int best_num_assignments;
    
    // Timed reports
    chrono::time_point<chrono::steady_clock> last_report;
};

};

#endif
