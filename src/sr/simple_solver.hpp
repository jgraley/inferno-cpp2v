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
    SimpleSolver( const list< shared_ptr<Constraint> > &constraints_ );

    void Start( const set<Value> &initial_domain_ );

    bool GetNextSolution( map< shared_ptr<Constraint>, list< Value > > *values = nullptr, 
                          SideInfo *side_info = nullptr );

private:
    typedef map<VariableId, Value> Assignments;

    static list<VariableId> DeduceVariables( const list< shared_ptr<Constraint> > &constraints );

    bool TryVariable( list<VariableId>::const_iterator current );
    bool Test( map<VariableId, Value> &assigns, 
               SideInfo *side_info = nullptr );
    list<Value> GetConstraintValues( shared_ptr<Constraint> c, const Assignments &a );
    
    // Set by constructor - depends on pattern only
    const list< shared_ptr<Constraint> > constraints;
    const list<VariableId> variables;

    // Used during solve - depends on pattern and x
    set<Value> initial_domain;    
    Assignments assignments;
    
    // Only needed to reserialise the matches TODO move to "holder" class
    list< pair<Assignments, shared_ptr<SideInfo> > > matches;
};

};

#endif
