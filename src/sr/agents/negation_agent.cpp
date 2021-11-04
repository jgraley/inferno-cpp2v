#include "negation_agent.hpp"
#include "conjecture.hpp"
#include "link.hpp"
#include "coupling.hpp"

using namespace SR;

shared_ptr<PatternQuery> NegationAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>(this);
	pq->RegisterAbnormalLink( PatternLink(this, GetPattern()) );
    
    auto evaluator = make_shared<BooleanEvaluator>( [](list<bool> inputs) -> bool
    {
        ASSERT( inputs.size() == 1 ); // we should remember one block
        return !inputs.front();        
    });
    
	pq->RegisterEvaluator( evaluator );
    return pq;
}


void NegationAgent::RunDecidedQueryPRed( DecidedQueryAgentInterface &query,
                                         XLink base_xlink ) const
{
    INDENT("¬");
    ASSERT( *GetPattern() );
    query.Reset();
    
    // Context is abnormal because patterns must not match
    query.RegisterAbnormalLink( PatternLink(this, GetPattern()), base_xlink ); // Link into X, abnormal
}


void NegationAgent::RunRegenerationQueryImpl( DecidedQueryAgentInterface &query,
                                              const SolutionMap *required_links,
                                              const TheKnowledge *knowledge ) const
{ 
    // This agent has no normal links, so just do this to populate query
    XLink base_xlink = required_links->at(base_plink);
    RunDecidedQueryPRed( query, base_xlink ); 
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
    block.title = "Negation";
	block.symbol = string("¬");
	block.shape = "diamond";
    block.block_type = Graphable::NODE_SHAPED;
    block.node = GetPatternPtr();
    auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(GetPattern()->get()), 
              list<string>{},
              list<string>{},
              phase,
              GetPattern() );
    block.sub_blocks = { { "pattern", 
                           "", 
                           true,
                           { link } } };
    return block;
}
