#include "agents/placeholder_agent.hpp"

using namespace SR;

shared_ptr<PatternQuery> PlaceholderAgent::GetPatternQuery() const
{
    ASSERTFAIL();
}


void PlaceholderAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                            XLink x ) const
{ 
    ASSERTFAIL();
}    


Graphable::Block PlaceholderAgent::GetGraphBlockInfo() const
{
	// The PlaceholderAgent node appears as a small circle with a . character inside it.
    // It should not appear in any graphs, however.
    Block block;
	block.bold = true;
	block.title = "Placeholder";
	block.symbol = ".";
	block.shape = "circle";
    block.block_type = Graphable::NODE;
    return block;
}
