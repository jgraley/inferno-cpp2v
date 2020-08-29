#include "simple_solver.hpp"
#include "query.hpp"
#include "agent.hpp"

using namespace SR;


void SimpleSolver::Configure( const set<Constraint *> &my_constraints_ ) :
    my_constraints(my_constraints_)
{
    for( Constraint *c : my_constraints )
    {
        list<VariableId> vars = c->GetVariables();
        
        for( VariableId v : vars )
        {
            my_variables.insert( v );
        }
    }
}

void SimpleSolver::Start()
{
}

bool SimpleSolver::GetNextSolution( std::list< TreePtr<Node> > *values = nullptr, 
                      Constraint::SideInfo *side_info = nullptr )
{
}