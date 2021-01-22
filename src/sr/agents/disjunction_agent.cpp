#include "disjunction_agent.hpp"
#include "conjecture.hpp"
#include "link.hpp"

using namespace SR;

void DisjunctionAgent::AgentConfigure( Phase phase, const SCREngine *master_scr_engine )
{
    AgentCommon::AgentConfigure(phase, master_scr_engine);

    options = make_shared< Collection<Node> >();
    FOREACH( const TreePtrInterface &p, GetPatterns() )
        options->insert( p );
}


shared_ptr<PatternQuery> DisjunctionAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>(this);
    pq->RegisterDecision(false); // Exclusive, please
    for( CollectionInterface::iterator pit = GetPatterns().begin(); pit != GetPatterns().end(); ++pit )                 
    {
        const TreePtrInterface *p = &*pit; 
	    pq->RegisterNormalLink( PatternLink(this, p) );
    }
    
    return pq;
}


void DisjunctionAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                            XLink x ) const
{
    INDENT("∨");
    ASSERT( !GetPatterns().empty() ); // must be at least one thing!
    query.Reset(); 
    
    // We register a decision that actually chooses between our agents - that
    // is, the options for the OR operation.
    ContainerInterface::iterator choice_it = query.RegisterDecision( options, false );
    FOREACH( const TreePtrInterface &p, GetPatterns() )                 
    {
        PatternLink plink(this, &p);

        // Context is normal because all patterns must match
        if( *plink.GetPatternPtr() == *choice_it ) // Is this the pattern that was chosen?
        {
            // Yes, so supply the "real" x for this link. We'll really
            // test x against this pattern.
            query.RegisterNormalLink( plink, x ); // Link into X
        }
        else
        {
            // No, so just make sure this link matches (overall AND-rule
            // applies, reducing the outcome to that of the normal 
            // link registered above).
            query.RegisterNormalLink( plink, XLink::MMAX_Link ); // Link into MMAX
        }
    }
}


bool DisjunctionAgent::ImplHasNLQ() const
{
    return true;
}


void DisjunctionAgent::RunNormalLinkedQueryImpl( PatternLink base_plink,
                                                 const SolutionMap *required_links,
                                                 const TheKnowledge *knowledge ) const
{ 
    INDENT("∨");
    bool found = false;
    bool complete = true;
    
    // Loop over the options for this disjunction
    FOREACH( const TreePtrInterface &p, GetPatterns() )           
    {
        PatternLink plink(this, &p);
        SolutionMap::const_iterator req_it = required_links->find(plink); // TODO hangover from when it was a list
        
        if( req_it == required_links->end() ) 
        {
            complete = false;
        }
        else
        {
            XLink req_xlink = req_it->second; 
            if( req_xlink == required_links->at(base_plink) )
                found = true; // This is the taken option
            else if( req_xlink != XLink::MMAX_Link )
                throw MMAXRequiredOnUntakenOptionMismatch();
        }        
    }
    
    // We only really have a mismatch if query was full i.e. we tried all the options
    if( complete && !found )
        throw NoOptionsMatchedMismatch();
}


Graphable::Block DisjunctionAgent::GetGraphBlockInfo() const
{
	// The Disjunction node appears as a diamond with a ∨ character inside it. The affected subtrees are 
	// on the right.
	// NOTE this node controls the action of the search engine in Inferno search/replace. It is not 
    // a node that represents a boolean operation in the program being processed. Those nodes would 
    // appear as rounded rectangles with the name at the top. Their names may be found in
	// src/tree/operator_db.txt  
    Block block;
	block.bold = true;
    block.title = "Disjunction";
	block.symbol = string("∨");
	block.shape = "diamond";
    block.block_type = Graphable::NODE;
    block.sub_blocks = { { "patterns", 
                           "", 
                           true,
                           {} } };
    FOREACH( const TreePtrInterface &p, GetPatterns() )
        block.sub_blocks.front().links.push_back( { (TreePtr<Node>)p, 
                                                    &p,
                                                    THROUGH, 
                                                    {},
                                                    {PatternLink(this, &p).GetShortName()} } );
    return block;
}
