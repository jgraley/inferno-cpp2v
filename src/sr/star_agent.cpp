#include "star_agent.hpp"
#include "helpers/simple_compare.hpp"
#include "search_replace.hpp" 

using namespace SR;

PatternQueryResult StarAgent::PatternQuery() const
{
    PatternQueryResult r;
    if( TreePtr<Node> p = GetRestriction() )
        r.AddLink( true, AsAgent(p) );
    return r;
}


// NOTE this is a DecidedCompare() not DecidedCompareImpl() so some of the AgentCommon 
// stuff has to be done explicitly in here.
void StarAgent::DecidedQuery( QueryAgentInterface &query,
                              const TreePtrInterface *px ) const
{
    INDENT("*");
    ASSERT(px);
    ASSERT(*px);
    query.Reset();
                
    ContainerInterface *xc = dynamic_cast<ContainerInterface *>(px->get());
    ASSERT(xc)("Nodes passed to StarAgent::DecidedCompare() must implement ContainerInterface, since * matches multiple things");
    
    // Check pre-restriction
    TRACE("MatchRange pre-res\n");
    FOREACH( TreePtr<Node> xe, *xc )
    {
        CheckLocalMatch( xe.get() );
    }
     
    if( TreePtr<Node> p = GetRestriction() )
    {
        TRACE("MatchRange pattern\n");
        // Apply pattern restriction - will be at least as strict as pre-restriction
        FOREACH( const TreePtrInterface &xe, *xc )
        {
            query.AddLocalLink( true, AsAgent(p), xe );
        }
    }
 
    TRACE("done\n");
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

