#include "negation_agent.hpp"
#include "conjecture.hpp"
#include "link.hpp"

using namespace SR;

shared_ptr<PatternQuery> NegationAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>(this);
	pq->RegisterAbnormalLink( PatternLink(this, GetPattern()) );
	pq->RegisterEvaluator( shared_ptr<BooleanEvaluator>( new BooleanEvaluatorNot() ) );
    return pq;
}


void NegationAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                         XLink x ) const
{
    INDENT("¬");
    ASSERT( *GetPattern() );
    query.Reset();
    
    // Check pre-restriction
    TRACE("Negation: local match check: ")(*this)(" vs ")(x)("\n");
    CheckLocalMatch(x.GetChildX().get());
    TRACE("Negation: local match check OK\n");
    
    // Context is abnormal because patterns must not match
    query.RegisterAbnormalLink( PatternLink(this, GetPattern()), x ); // Link into X, abnormal
    query.RegisterEvaluator( shared_ptr<BooleanEvaluator>( new BooleanEvaluatorNot() ) );
}


Graphable::Block NegationAgent::GetGraphBlockInfo() const
{
	// The Negation node appears as a diamond with a ¬ character inside it. The affected subtree is 
	// on the right.
	// NOTE this node controls the action of the search engine in Inferno search/replace. It is not 
    // a node that represents a boolean operation in the program being processed. Those nodes would 
    // appear as rounded rectangles with the name at the top. Their names may be found in
	// src/tree/operator_db.txt  
    Block block;
	block.bold = true;
	block.title = string("¬");
	block.shape = "diamond";
    block.block_type = Graphable::NODE;
    block.sub_blocks = { { "pattern", 
                           "", 
                           true,
                           { { (TreePtr<Node>)*GetPattern(),
                               GetPattern(), 
                               THROUGH, 
                               {},
                               {PatternLink(this, GetPattern()).GetShortName()} } } } };
    return block;
}


bool NegationAgent::BooleanEvaluatorNot::operator()( list<bool> &inputs ) const
{
	ASSERT( inputs.size() == 1 ); // we should remember one block
	return !inputs.front();
}
