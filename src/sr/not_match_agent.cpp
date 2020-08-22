#include "not_match_agent.hpp"
#include "conjecture.hpp"

using namespace SR;

shared_ptr<PatternQuery> NotMatchAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
	pq->RegisterAbnormalLink( AsAgent(GetPattern()) );
	pq->RegisterEvaluator( shared_ptr<BooleanEvaluator>( new BooleanEvaluatorNot() ) );
    return pq;
}


void NotMatchAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                  const TreePtrInterface *px ) const
{
    INDENT("!");
    ASSERT( GetPattern() );
    query.Reset();
    
    // Check pre-restriction
    CheckLocalMatch(px->get());
    
    // Context is abnormal because patterns must not match
    query.RegisterAbnormalLink( AsAgent(GetPattern()), px );
    query.RegisterEvaluator( shared_ptr<BooleanEvaluator>( new BooleanEvaluatorNot() ) );
}


void NotMatchAgent::GetGraphAppearance( bool *bold, string *text, string *shape ) const
{
	// The NotMatch node appears as a small circle with an ! character inside it. The affected subtree is 
	// on the right.
	// NOTE this and the next few special nodes are the nodes that control the action of the search engine in 
	// Inferno search/replace. They are not the nodes that represent the operations in the program being processed.
	// Those nodes would appear as rounded rectangles with the name at the top. The nmes may be found in
	// src/tree/operator_db.txt  
	*bold = true;
	*shape = "circle";
	*text = string("!");
}


bool NotMatchAgent::BooleanEvaluatorNot::operator()( list<bool> &inputs ) const
{
	ASSERT( inputs.size() == 1 ); // we should remember one block
	return !inputs.front();
}
