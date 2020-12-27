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
                                     XLink x ) const
{
    INDENT("*");
    ASSERT(x);
    query.Reset();
                
    ContainerInterface *x_ci = dynamic_cast<ContainerInterface *>(x.GetChildX().get());
    auto x_sc = TreePtr<SubContainer>::DynamicCast(x.GetChildX());

    // Nodes passed to StarAgent::RunDecidedQueryImpl() must be a SubContainer, since * matches multiple things
    if( !( x_sc && x_ci ) )
        throw Mismatch();
    
    // Check pre-restriction
    TRACE("StarAgent pre-res\n");
    FOREACH( const TreePtrInterface &xe, *x_ci )
    {
        CheckLocalMatch( ((TreePtr<Node>)xe).get() );
    }
     /* For #207
    for( XLink elt_xlink : x_ci->elts )
    {
        CheckLocalMatch( elt_xlink.GetChildX().get() );
    }
    */
     
    if( *GetRestriction() )
    {
        TRACE("StarAgent pattern, size is %d\n", x_ci->size());
        // Apply pattern restriction - will be at least as strict as pre-restriction
        query.RegisterMultiplicityLink( PatternLink(this, GetRestriction()), x ); // Links into X
    }
}                       


TreePtr<Node> StarAgent::BuildReplaceImpl( CouplingKey keylink ) 
{
    INDENT("*");
    ASSERT( keylink );
    TreePtr<Node> keynode = keylink.GetChildX();
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
	block.title = string("*");
	block.shape = "circle";
    block.block_type = Graphable::NODE;
    if( *GetRestriction() )
        block.sub_blocks.push_back( { "restriction", 
                                      "", 
                                      { { (TreePtr<Node>)*GetRestriction(), 
                                          SOLID, 
                                          {},
                                          {PatternLink(this, GetRestriction()).GetShortName()} } } } );
    return block;
}

