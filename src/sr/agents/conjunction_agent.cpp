#include "conjunction_agent.hpp"
#include "conjecture.hpp"
#include "link.hpp"

using namespace SR;


shared_ptr<PatternQuery> ConjunctionAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>(this);
    // TODO don't really need iterator, so could use FOREACH, and in DQ()
    for( CollectionInterface::iterator pit = GetPatterns().begin(); pit != GetPatterns().end(); ++pit )                 
    {
        const TreePtrInterface *p = &*pit; 
	    pq->RegisterNormalLink( PatternLink(this, p) );
    }
        
    return pq;
}


Graphable::Block ConjunctionAgent::GetGraphBlockInfo( const LinkNamingFunction &lnf ) const
{
	// The Conjunction node appears as a diamond with a ∧ character inside it. The affected subtrees are 
	// on the right.
	// NOTE this node controls the action of the search engine in Inferno search/replace. It is not 
    // a node that represents a boolean operation in the program being processed. Those nodes would 
    // appear as rounded rectangles with the name at the top. Their names may be found in
	// src/tree/operator_db.txt  
    Block block;
	block.bold = true;
    block.title = "Conjunction";
	block.symbol = string("∧");
	block.shape = "diamond";
    block.block_type = Graphable::NODE;
    block.sub_blocks = { { "patterns", 
                           "", 
                           true,
                           {} } };
    FOREACH( const TreePtrInterface &p, GetPatterns() )
        block.sub_blocks.front().links.push_back( { &p,
                                                    THROUGH, 
                                                    {},
                                                    {PatternLink(this, &p).GetShortName()} } );
    return block;
}
