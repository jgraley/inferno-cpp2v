#ifndef SIMPLE_SOLVER
#define SIMPLE_SOLVER

#include "constraint.hpp"

#include "node/node.hpp"
#include "common/common.hpp"

namespace CSP
{ 
    
class Agent;
    
/** A simple back-tracking solver
 */
class SimpleSolver
{
public:
    SimpleSolver( const list<Constraint *> &constraints_, 
                  const list<Value> &initial_domain_ );

    void Start();

    bool GetNextSolution( map< Constraint *, list< Value > > *values = nullptr, 
                          SideInfo *side_info = nullptr );

private:
    typedef map<VariableId, Value> Assignments;

    static list<VariableId> DeduceVariables( const list<Constraint *> &constraints );

    bool TryVariable( list<VariableId>::const_iterator current );
    bool Test( map<VariableId, Value> &assigns, 
               SideInfo *side_info = nullptr );
    list<Value> GetConstraintValues( const Constraint *c, const Assignments &a );
    
    const list<Constraint *> constraints;
    const list<Value> initial_domain;
    const list<VariableId> variables;
    
    Assignments assignments;
    list< pair<Assignments, SideInfo> > matches;
    list< pair<Assignments, SideInfo> >::const_iterator next_match;
};

};

#endif
