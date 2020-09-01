#ifndef SIMPLE_SOLVER_HPP
#define SIMPLE_SOLVER_HPP

#include "constraint.hpp"
#include "solver.hpp"

#include "node/node.hpp"
#include "common/common.hpp"

namespace CSP
{ 
    
class Agent;
class SolverHolder;
    
/** A simple back-tracking solver
 */
class SimpleSolver : public Solver
{
public:
    SimpleSolver( const list< shared_ptr<Constraint> > &constraints );

    void Run( ReportageObserver *holder, const set<Value> &initial_domain );

private:
    typedef map<VariableId, Value> Assignments;

    static list<VariableId> DeduceVariables( const list< shared_ptr<Constraint> > &constraints );

    bool TryVariable( list<VariableId>::const_iterator current );
    bool Test( map<VariableId, Value> &assigns, 
               SideInfo *side_info = nullptr );
    list<Value> GetConstraintValues( shared_ptr<Constraint> c, const Assignments &a );    
    void ReportSolution( const Assignments &assignments, 
                         shared_ptr<SideInfo> side_info );
    
    ReportageObserver *holder;
        
    // Set by constructor - depends on pattern only
    const list< shared_ptr<Constraint> > constraints;
    const list<VariableId> variables;

    // Used during solve - depends on pattern and x
    set<Value> initial_domain;    
    Assignments assignments;
};

};

#endif
