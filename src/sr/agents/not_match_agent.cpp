#include "not_match_agent.hpp"
#include "conjecture.hpp"
#include "link.hpp"

using namespace SR;

shared_ptr<PatternQuery> NotMatchAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>(this);
	pq->RegisterAbnormalLink( PatternLink(this, GetPattern()) );
	pq->RegisterEvaluator( shared_ptr<BooleanEvaluator>( new BooleanEvaluatorNot() ) );
    return pq;
}


void NotMatchAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                         XLink x ) const
{
    INDENT("!");
    ASSERT( *GetPattern() );
    query.Reset();
    
    // Check pre-restriction
    TRACE("NotMatch: local match check: ")(*this)(" vs ")(x)("\n");
    CheckLocalMatch(x.GetChildX().get());
    TRACE("NotMatch: local match check OK\n");
    
    // Context is abnormal because patterns must not match
    query.RegisterAbnormalLink( PatternLink(this, GetPattern()), x ); // Link into X, abnormal
    query.RegisterEvaluator( shared_ptr<BooleanEvaluator>( new BooleanEvaluatorNot() ) );
}


Graphable::Block NotMatchAgent::GetGraphBlockInfo() const
{
	// The NotMatch node appears as a small circle with an ! character inside it. The affected subtree is 
	// on the right.
	// NOTE this node controls the action of the search engine in Inferno search/replace. It is not 
    // a node that represents a boolean operation in the program being processed. Those nodes would 
    // appear as rounded rectangles with the name at the top. Their names may be found in
	// src/tree/operator_db.txt  
    Block block;
	block.bold = true;
	block.title = string("!");
	block.shape = "circle";
    block.block_type = Graphable::NODE;
    block.sub_blocks = { { "pattern", 
                           "", 
                           { { (TreePtr<Node>)*GetPattern(), 
                               SOLID, 
                               {},
                               {PatternLink(this, GetPattern()).GetShortName()} } } } };
    return block;
}


bool NotMatchAgent::BooleanEvaluatorNot::operator()( list<bool> &inputs ) const
{
	ASSERT( inputs.size() == 1 ); // we should remember one block
	return !inputs.front();
}
