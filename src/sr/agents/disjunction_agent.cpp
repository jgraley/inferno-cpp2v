#include "disjunction_agent.hpp"
#include "conjecture.hpp"
#include "link.hpp"

using namespace SR;

void DisjunctionAgent::SCRConfigure( const SCREngine *e,
                                     Phase phase )
{
    AgentCommon::SCRConfigure(e, phase);

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
    
    /*if( x == XLink::MMAX_Link )
    {
        // Magic Match Anything node: all normal children also match anything
        // This is just to keep normal-domain solver happy, so we 
        // only need normals. 
        for( PatternLink plink : pattern_query->GetNormalLinks() )       
            query.RegisterNormalLink( plink, x );
        return;
    } 
*/    
    // Check pre-restriction
    if( !IsLocalMatch( x.GetChildX().get() ) )
        throw PreRestrictionMismatch();

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


void DisjunctionAgent::RunNormalLinkedQuery( const SolutionMap *required_links,
                                             const TheKnowledge *knowledge ) const
{ 
    // Baseless query strategy: hand-rolled
    INDENT("∨");
    XLink base_xlink;
    if( required_links->count(keyer_plink) > 0 )
    { 
        base_xlink = required_links->at(keyer_plink);
        
        // Check pre-restriction
        if( base_xlink == XLink::MMAX_Link && !IsLocalMatch( base_xlink.GetChildX().get() ) )
            throw PreRestrictionMismatch();
    }
           
    // Loop over the options for this disjunction and collect the links 
    // that are not MMAX. Also take note of missing children.
    bool children_complete = true;
    list<XLink> non_mmax_residuals;
    FOREACH( const TreePtrInterface &p, GetPatterns() )           
    {
        PatternLink plink(this, &p);
        
        if( required_links->count(plink) > 0 ) 
        {
            XLink xlink = required_links->at(plink); 
            ASSERT( xlink );
            if( xlink != XLink::MMAX_Link )
                non_mmax_residuals.push_back( xlink );
        }        
        else
        {
            children_complete = false;
        }        
    }
    
    if( base_xlink == XLink::MMAX_Link )
    {
        // Choose a checking strategy based on the number of non-MMAX residuals we saw. 
        // Roughly speaking, it should be 0, but see the code for details.
        switch( non_mmax_residuals.size() )
        {
        case 0:
            // All were MMAX: great! 
            break;        
            
        default: // 1 or more
            // It's never OK to have non-MMAX under MMAX.
            throw MMAXPropagationMismatch();        
            break;
        }    
    }
    else
    {
        // Choose a checking strategy based on the number of non-MMAX residuals we saw. 
        // Roughly speaking, it should be 1, but see the code for details.
        switch( non_mmax_residuals.size() )
        {
        case 0:
            // All were MMAX: we only have a mismatch if query was full i.e. we tried all the options
            if( children_complete )
                throw NoOptionsMatchedMismatch();    
            break;        
            
        case 1:
            // This is the correct number of non-MMAX. If we have a base, check against it.
            if( base_xlink )
            {
                XLink taken_option_x_link = non_mmax_links.front(); // size() is 1 so is the only one
                if( taken_option_x_link != base_xlink )
                    throw TakenOptionMismatch();  
            }
            break;        
            
        default: // 2 or more
            // It's never OK to have more than one non-MMAX (strict MMAX rules).
            throw MMAXRequiredOnUntakenOptionMismatch();        
            break;
        }    
    }
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
    block.block_type = Graphable::NODE_SHAPED;
    block.node = GetPatternPtr();
    block.sub_blocks = { { "patterns", 
                           "", 
                           true,
                           {} } };
    FOREACH( const TreePtrInterface &p, GetPatterns() )
    {
        auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(p.get()),
                  list<string>{},
                  list<string>{},
                  phase,
                  &p );
        block.sub_blocks.front().links.push_back( link );
    }

    return block;
}
