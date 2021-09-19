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


void DisjunctionAgent::RunDecidedQueryPRed( DecidedQueryAgentInterface &query,
                                            XLink x ) const
{
    INDENT("∨");
    ASSERT( !GetPatterns().empty() ); // must be at least one thing!
    
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

    
bool DisjunctionAgent::NLQRequiresBase() const
{
    return false;
}                                         


void DisjunctionAgent::RunNormalLinkedQueryPRed( const SolutionMap *required_links,
                                                 const TheKnowledge *knowledge ) const
{ 
    // Baseless query strategy: hand-rolled
    INDENT("∨");
    XLink base_xlink;
    if( required_links->count(base_plink) > 0 )
        base_xlink = required_links->at(base_plink);
    
    ASSERT( base_xlink != XLink::MMAX_Link ); // DefaultMMAXAgent should have taken care of this case
       
    // Loop over the options for this disjunction and collect the links 
    // that are not MMAX. Also take note of missing children.
    bool children_complete = true;
    list<XLink> non_mmax_links;
    FOREACH( const TreePtrInterface &p, GetPatterns() )           
    {
        PatternLink plink(this, &p);
        
        if( required_links->count(plink) > 0 ) 
        {
            XLink xlink = required_links->at(plink); 
            ASSERT( xlink );
            if( xlink != XLink::MMAX_Link )
                non_mmax_links.push_back( xlink );
        }        
        else
        {
            children_complete = false;
        }        
    }
    
    // Choose a checking strategy based on the number of non-MMAX we saw. 
    // Roughly speaking, it should be 1, but see the code for details.
    switch( non_mmax_links.size() )
    {
    case 0:
        // All were MMAX: we only have a mismatch if query was full i.e. we tried all the options
        if( children_complete )
            throw NoOptionsMatchedMismatch();    
        break;        
        
    case 1:
        // This is the correct number of non-MMAX. If we have a base, check against it.
        if( base_xlink && non_mmax_links.front() != base_xlink )
            throw TakenOptionMismatch();  
        break;        
        
    default: // 2 or more
        // It's never OK to have more than one non-MMAX.
        throw MMAXRequiredOnUntakenOptionMismatch();        
        break;
    }    
}


Graphable::Block DisjunctionAgent::GetGraphBlockInfo( const LinkNamingFunction &lnf,
                                     const NonTrivialPreRestrictionFunction &ntprf ) const
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
    {
        auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(p.get()),
                  list<string>{},
                  list<string>{PatternLink(this, &p).GetShortName()},
                  phase,
                  SpecialBase::IsNonTrivialPreRestriction(&p) );
        block.sub_blocks.front().links.push_back( link );
    }

    return block;
}
