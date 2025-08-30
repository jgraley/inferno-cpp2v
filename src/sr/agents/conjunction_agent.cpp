#include "conjunction_agent.hpp"
#include "conjecture.hpp"
#include "link.hpp"

using namespace SR;


shared_ptr<PatternQuery> ConjunctionAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
    for( const TreePtrInterface &p : GetConjuncts() )                 
        pq->RegisterNormalLink( PatternLink(&p) );
        
    return pq;
}


Graphable::NodeBlock ConjunctionAgent::GetGraphBlockInfo() const
{
    // The Conjunction node appears as a diamond with a ∧ character inside it. The affected subtrees are 
    // on the right.
    // NOTE this node controls the action of the search engine in Inferno search/replace. It is not 
    // a node that represents a boolean operation in the program being processed. Those nodes would 
    // appear as rounded rectangles with the name at the top. Their names may be found in
    // src/tree/operator_db.txt  
    NodeBlock block;
    block.bold = true;
    block.title = "Conjunction";
    block.symbol = string("∧");
    block.shape = "diamond";
    block.block_type = Graphable::NODE_SHAPED;
    block.node = GetPatternPtr();
    block.item_blocks = { { "patterns", 
                           "", 
                           true,
                           {} } };
    for( const TreePtrInterface &p : GetConjuncts() )
    {
        auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(p.get()),
                  list<string>{},
                  list<string>{},
                  phase,
                  &p );
        block.item_blocks.front().links.push_back( link );
    }
    return block;
}


Agent::ReplacePatchPtr ConjunctionAgent::GenReplaceLayoutImpl( const ReplaceKit &kit, 
                                                               PatternLink me_plink, 
                                                               XLink key_xlink,
                                                               const SCREngine *acting_engine )
{
	(void)me_plink;
	(void)key_xlink;
	shared_ptr<PatternQuery> pq = GetPatternQuery();
    auto plinks = pq->GetNormalLinks();
    ASSERT( plinks.size() >= 1 );

	if( plinks.size() >= 2 )
		FTRACE("Warning, replace is choosing first of:\n")(plinks)("\n");

    // Pick the first normal plink
    PatternLink replace_plink = plinks.front();
    ASSERT( replace_plink );          
    return replace_plink.GetChildAgent()->GenReplaceLayout(kit, replace_plink, acting_engine);    
} 
