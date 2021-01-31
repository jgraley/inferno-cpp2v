#ifndef SIMPLE_SOLVER_HPP
#define SIMPLE_SOLVER_HPP

#include "constraint.hpp"
#include "solver.hpp"

#include "node/specialise_oostd.hpp"
#include "common/common.hpp"

#include <chrono>
#include <functional>

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
              const Assignments &forces,
              const SR::TheKnowledge *knowledge );

private:
    typedef set<shared_ptr<Constraint>> ConstraintSet;

    const struct Plan
    {
        Plan( const list< shared_ptr<Constraint> > &constraints, 
              const list<VariableId> *variables = nullptr );
        void DeduceVariables( const list<VariableId> *variables );
    
        list<VariableId> variables;
        list< shared_ptr<Constraint> > constraints;

        ConstraintSet constraint_set;
        map<VariableId, ConstraintSet> affected_constraints;
    } plan;

    void Solve( list<VariableId>::const_iterator current_it );

    class ValueSelector : public Traceable
    {
    public:
        ValueSelector( const Plan &solver_plan,
                       const SimpleSolver &solver,
                       const SR::TheKnowledge *knowledge,
                       Assignments &assignments,
                       list<VariableId>::const_iterator current_it );
        ~ValueSelector();
        Value SelectNextValue();
        
    private:
        const Plan &solver_plan;
        const SimpleSolver &solver;
        const SR::TheKnowledge * const knowledge;
        Assignments &assignments;
        const list<VariableId>::const_iterator current_it;
        const VariableId current_var;
        list<Value> value_queue;    
    };

    pair<bool, Assignment> Test( const Assignments &assigns, const ConstraintSet &to_test ) const;
    void TraceProblem() const;
    static void CheckLocalMatch( const Assignments &assignments, VariableId variable );
    void ShowBestAssignment();
    void TimedOperations();
    void CheckPlanVariablesUsed();

    // Structural
    ReportageObserver *holder;    
        
    // Used during solve - depends on pattern and x
    const SR::TheKnowledge *knowledge;
    Assignments assignments;
    map< VariableId, shared_ptr<ValueSelector> > value_selectors;
    
    // Timed reports
    chrono::time_point<chrono::steady_clock> last_report;
};

};

#endif
