#include "agent.hpp"

#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "scr_engine.hpp"
#include "and_rule_engine.hpp"
#include "link.hpp"

// Temporary
#include "tree/cpptree.hpp"

#include "transform_of_agent.hpp"
#include "sym/lambdas.hpp"
#include "sym/boolean_operators.hpp"
#include "sym/comparison_operators.hpp"
#include "sym/primary_expressions.hpp"
#include "sym/rewriters.hpp"

#include <stdexcept>

using namespace SR;
using namespace SYM;

//---------------------------------- Agent ------------------------------------    

// C++11 fix
Agent& Agent::operator=(Agent& other)
{
    (void)Node::operator=(other); 
    return *this;
}


Agent *Agent::AsAgent( shared_ptr<Node> node )
{
    Agent *agent = TryAsAgent(node);
    ASSERTS( agent )("Called AsAgent(")(*node)(") with non-Agent");
    return agent;
}


Agent *Agent::TryAsAgent( shared_ptr<Node> node )
{
    ASSERTS( node )("Called TryAsAgent(")(node)(") with null shared_ptr");
    return dynamic_cast<Agent *>(node.get());
}


const Agent *Agent::AsAgentConst( shared_ptr<const Node> node )
{
    const Agent *agent = TryAsAgentConst(node);
    ASSERTS( agent )("Called AsAgent(")(*node)(") with non-Agent");
    return agent;
}


const Agent *Agent::TryAsAgentConst( shared_ptr<const Node> node )
{
    ASSERTS( node )("Called TryAsAgent(")(node)(") with null shared_ptr");
    return dynamic_cast<const Agent *>(node.get());
}


