#include "star_agent.hpp"
#include "helpers/simple_compare.hpp"
#include "search_replace.hpp" 

using namespace SR;

// NOTE this is a DecidedCompare() not DecidedCompareImpl() so some of the AgentCommon 
// stuff has to be done explicitly in here.
bool StarAgent::DecidedQueryImpl( const TreePtrInterface &x ) const
{
    INDENT("*");
    ASSERT(x);
                
    ContainerInterface *xc = dynamic_cast<ContainerInterface *>(x.get());
    ASSERT(xc)("Nodes passed to StarAgent::DecidedCompare() must implement ContainerInterface, since * matches multiple things");
    
    // Check pre-restriction
    TRACE("MatchRange pre-res\n");
    FOREACH( TreePtr<Node> xe, *xc )
    {
        if( !IsLocalMatch( xe.get()) )
            return false;
    }
     
    if( TreePtr<Node> p = GetRestriction() )
    {
        TRACE("MatchRange pattern\n");
        // Apply pattern restriction - will be at least as strict as pre-restriction
        FOREACH( const TreePtrInterface &xe, *xc )
        {
            RememberLocalLink( true, AsAgent(p), xe );
        }
    }
 
    TRACE("done\n");
    return true;
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


