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
    void Configure( const std::set<Constraint *> &my_constraints_ );

    void Start();

    bool GetNextSolution( std::list< TreePtr<Node> > *values = nullptr, 
                          Constraint::SideInfo *side_info = nullptr );

private:
    std::set<Constraint *> my_constraints;
    std::set<Constraint::VariableId> my_variables;
};

};

#endif
