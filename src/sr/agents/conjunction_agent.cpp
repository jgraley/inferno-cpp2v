#include "conjunction_agent.hpp"
#include "conjecture.hpp"
#include "link.hpp"

using namespace SR;

shared_ptr<PatternQuery> ConjunctionAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>(this);
    for( CollectionInterface::iterator pit = GetPatterns().begin(); pit != GetPatterns().end(); ++pit )                 
    {
        const TreePtrInterface *p = &*pit; 
	    pq->RegisterNormalLink( PatternLink(this, p) );
    }
        
    return pq;
}


void ConjunctionAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                         XLink x ) const
{ 
    INDENT("∧");
    ASSERT( !GetPatterns().empty() ); // must be at least one thing!
    query.Reset();
    
    // Check pre-restriction
    CheckLocalMatch(x.GetChildX().get());
    
    for( CollectionInterface::iterator pit = GetPatterns().begin(); pit != GetPatterns().end(); ++pit )                 
    {
        const TreePtrInterface *p = &*pit; 
        ASSERT( *p );
        // Context is normal because all patterns must match (but none should contain
        // nodes with reploace functionlity because they will not be invoked during replace) 
        query.RegisterNormalLink( PatternLink(this, p), x ); // Link into X
    }
}    


Graphable::Block ConjunctionAgent::GetGraphBlockInfo() const
{
	// The Conjunction node appears as a small circle with an & character inside it. The affected subtrees are 
	// on the right.
	// NOTE this node controls the action of the search engine in Inferno search/replace. It is not 
    // a node that represents a boolean operation in the program being processed. Those nodes would 
    // appear as rounded rectangles with the name at the top. Their names may be found in
	// src/tree/operator_db.txt  
    Block block;
	block.bold = true;
	block.title = string("∧");
	block.shape = "circle";
    block.block_type = Graphable::NODE;
    block.sub_blocks = { { "patterns", 
                           "", 
                           {} } };
    FOREACH( const TreePtrInterface &p, GetPatterns() )
        block.sub_blocks.front().links.push_back( { (TreePtr<Node>)p, 
                                                    &p,
                                                    SOLID, 
                                                    {},
                                                    {PatternLink(this, &p).GetShortName()} } );
    return block;
}