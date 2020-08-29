#ifndef SIMPLE_SOLVER
#define SIMPLE_SOLVER

#include "constraint.hpp"

#include "node/node.hpp"
#include "common/common.hpp"

namespace SR
{ 
    
class Agent;
    
/** A simple back-tracking solver
 */
class SimpleSolver
{
public:
    SimpleSolver( const std::list<Constraint *> &constraints_, 
                  const std::list<Constraint::Value> &initial_domain_ );

    void Start();

    bool GetNextSolution( std::list< TreePtr<Node> > *values = nullptr, 
                          Constraint::SideInfo *side_info = nullptr );

private:
    static std::list<Constraint::VariableId> DeduceVariables( const std::list<Constraint *> &constraints );

    bool Test( std::map<Constraint::VariableId, Constraint::Value> &assigns );

    const std::list<Constraint *> constraints;
    const std::list<Constraint::Value> initial_domain;
    const std::list<Constraint::VariableId> variables;
    
    std::map<Constraint::VariableId, Constraint::Value> assignments;
    std::list<Constraint::VariableId>::const_iterator current;
};

};

#endif
