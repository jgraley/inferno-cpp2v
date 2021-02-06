#include "star_agent.hpp"
#include "../subcontainers.hpp" 
#include "../search_replace.hpp" 
#include "link.hpp"

using namespace SR;


shared_ptr<PatternQuery> StarAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>(this);
    if( *GetRestriction() )
    {
        pq->RegisterMultiplicityLink( PatternLink(this, GetRestriction()) );
    }

    return pq;
}


// NOTE this is a DecidedCompare() not DecidedCompareImpl() so some of the AgentCommon 
// stuff has to be done explicitly in here.
void StarAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                     XLink base_xlink ) const
{
    INDENT("*");
    ASSERT(base_xlink);
                
    auto x_ci = dynamic_cast<ContainerInterface *>(base_xlink.GetChildX().get());
    auto x_sc = TreePtr<SubContainer>::DynamicCast(base_xlink.GetChildX());

    // Nodes passed to StarAgent::RunDecidedQueryMMed() must be a SubContainer, since * matches multiple things
    if( !( x_sc && x_ci ) )
        throw NotASubcontainerMismatch();
    
    // Check pre-restriction
    TRACE("StarAgent pre-res\n");
    FOREACH( const TreePtrInterface &xe, *x_ci )
    {
        if( !IsLocalMatch( ((TreePtr<Node>)xe).get() ) )
            throw PreRestrictionMismatch();
    }
     /* For #207
    for( XLink elt_xlink : x_ci->elts )
    {
        if( !IsLocalMatch( elt_xlink.GetChildX().get() ) )
            throw PreRestrictionMismatch();
    }
    */
     
    if( *GetRestriction() )
    {
        TRACE("StarAgent pattern, size is %d\n", x_ci->size());
        // Apply pattern restriction - will be at least as strict as pre-restriction
        query.RegisterMultiplicityLink( PatternLink(this, GetRestriction()), base_xlink ); // Links into X
    }
}                       


void StarAgent::RunRegenerationQueryImpl( DecidedQueryAgentInterface &query,
                                          XLink base_xlink,
                                          const SolutionMap *required_links,
                                          const TheKnowledge *knowledge ) const
{ 
    // This agent has no normal links, so just do this to populate query
    RunDecidedQueryImpl( query, base_xlink ); 
}


TreePtr<Node> StarAgent::BuildReplaceImpl() 
{
    INDENT("*");
    ASSERT( GetKey() );
    TreePtr<Node> keynode = GetKey().GetChildX();
    ContainerInterface *psc = dynamic_cast<ContainerInterface *>(keynode.get());
    ASSERT( psc )("Star node ")(*this)(" keyed to ")(*keynode)(" which should implement ContainerInterface");  
    TRACE("Walking container length %d\n", psc->size() );
    
    if( auto sc = dynamic_cast<SubContainer *>(keynode.get()) )    
        TRACE("SubContainer found ")(sc->GetContentsTrace())("\n");
        
    TreePtr<SubContainer> dest;
    ContainerInterface *dest_container;
    if( dynamic_cast<SequenceInterface *>(keynode.get()) )
        dest = TreePtr<SubSequence>(new SubSequence);
    else if( dynamic_cast<CollectionInterface *>(keynode.get()) )
        dest = TreePtr<SubCollection>(new SubCollection);
    else
        ASSERT(0)("Please add new kind of Star");
    
    dest_container = dynamic_cast<ContainerInterface *>(dest.get());
    FOREACH( const TreePtrInterface &pp, *psc )
    {
        TRACE("Building ")(pp)("\n");
        TreePtr<Node> nn = DuplicateSubtree( (TreePtr<Node>)pp );
        dest_container->insert( nn );
    }
    
    return dest;
}


Graphable::Block StarAgent::GetGraphBlockInfo() const
{
	// The Star node appears as a small circle with a * character inside it. * is chosen for its role in 
	// filename wildcarding, which is semantically equiviant only when used in a Sequence.
    Block block;
	block.bold = true;
	block.title = "Star";
	block.symbol = "*";
	block.shape = "circle";
    block.block_type = Graphable::NODE;
    if( *GetRestriction() )
        block.sub_blocks.push_back( { "restriction", 
                                      "", 
                                      false,
                                      { { (TreePtr<Node>)*GetRestriction(), 
                                          GetRestriction(),
                                          THROUGH, 
                                          {},
                                          {PatternLink(this, GetRestriction()).GetShortName()} } } } );
    return block;
}

