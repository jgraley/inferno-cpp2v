#include "placeholder_agent.hpp"

using namespace SR;

shared_ptr<PatternQuery> PlaceholderAgent::GetPatternQuery() const
{
    ASSERTFAIL();
}


void PlaceholderAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                            TreePtr<Node> x ) const
{ 
    ASSERTFAIL();
}    


void PlaceholderAgent::GetGraphAppearance( bool *bold, string *text, string *shape ) const
{
	// The PlaceholderAgent node appears as a small circle with a . character inside it.
    // It should not appear in any graphs, however.
	*bold = true;
	*shape = "circle";
	*text = string(".");
}
