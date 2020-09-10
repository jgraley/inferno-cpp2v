#include "star_agent.hpp"
#include "helpers/simple_compare.hpp"
#include "subcontainers.hpp" 
#include "search_replace.hpp" 

using namespace SR;


shared_ptr<PatternQuery> StarAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
    if( *GetRestriction() )
    {
        pq->RegisterMultiplicityLink( GetRestriction() );
    }

    return pq;
}


// NOTE this is a DecidedCompare() not DecidedCompareImpl() so some of the AgentCommon 
// stuff has to be done explicitly in here.
void StarAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                     TreePtr<Node> x ) const
{
    INDENT("*");
    ASSERT(x);
    query.Reset();
                
    ContainerInterface *x_ci = dynamic_cast<ContainerInterface *>(x.get());
    auto x_sc = TreePtr<SubContainer>::DynamicCast(x);

    // Nodes passed to StarAgent::RunDecidedQueryImpl() must be a SubContainer, since * matches multiple things
    if( !( x_sc && x_ci ) )
        throw Mismatch();
    
    // Check pre-restriction
    TRACE("StarAgent pre-res\n");
    FOREACH( TreePtr<Node> xe, *x_ci )
    {
        CheckLocalMatch( xe.get() );
    }
     
    if( *GetRestriction() )
    {
        TRACE("StarAgent pattern, size is %d\n", x_ci->size());
        // Apply pattern restriction - will be at least as strict as pre-restriction
        
        query.RegisterMultiplicityLink( GetRestriction(), x_sc ); // Links into X
    }
}                       


TreePtr<Node> StarAgent::BuildReplaceImpl( TreePtr<Node> keynode ) 
{
    INDENT("*");
    ASSERT( keynode );
    ContainerInterface *psc = dynamic_cast<ContainerInterface *>(keynode.get());
    ASSERT( psc )("Star node ")(*this)(" keyed to ")(*keynode)(" which should implement ContainerInterface");  
    TRACE("Walking container length %d\n", psc->size() );
    
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
        TreePtr<Node> nn = DuplicateSubtree( pp );
        dest_container->insert( nn );
    }
    
    return dest;
}


void StarAgent::GetGraphAppearance( bool *bold, string *text, string *shape ) const
{
	// The Star node appears as a small circle with a * character inside it. * is chosen for its role in 
	// filename wildcarding, which is semantically equiviant only when used in a Sequence.
	*bold = true;
	*shape = "circle";
	*text = string("*");
}

