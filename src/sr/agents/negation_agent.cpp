#include "negation_agent.hpp"
#include "conjecture.hpp"
#include "link.hpp"
#include "sym/boolean_operators.hpp"
#include "sym/predicate_operators.hpp"
#include "sym/symbol_operators.hpp"
#include "sym/lazy_eval.hpp"

using namespace SR;
using namespace SYM;

shared_ptr<PatternQuery> NegationAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
	pq->RegisterAbnormalLink( PatternLink(this, GetNegand()) );
    
    auto evaluator = make_shared<BooleanEvaluator>( [this](list<bool> inputs) -> bool
    {
        ASSERT( inputs.size() == 1 ); // we should remember one block
        return !inputs.front();        
    });
    
	pq->RegisterEvaluator( evaluator );
    return pq;
}


SYM::Lazy<SYM::BooleanExpression> NegationAgent::SymbolicNormalLinkedQueryPRed() const
{
    return MakeLazy<SYM::BooleanConstant>(true);
}                                      


void NegationAgent::RunRegenerationQueryImpl( DecidedQueryAgentInterface &query,
                                              const SolutionMap *hypothesis_links,
                                              const XTreeDatabase *x_tree_db ) const
{ 
    ASSERT( *GetNegand() );
    // This agent has no normal links, so just do this to populate query
    XLink keyer_xlink = hypothesis_links->at(keyer_plink);
    query.Reset();
    
    // Context is abnormal because patterns must not match
    query.RegisterAbnormalLink( PatternLink(this, GetNegand()), keyer_xlink ); // Link into X, abnormal
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
    auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(GetNegand()->get()), 
              list<string>{},
              list<string>{},
              phase,
              GetNegand() );
    block.sub_blocks = { { "pattern", 
                           "", 
                           true,
                           { link } } };
    return block;
}
