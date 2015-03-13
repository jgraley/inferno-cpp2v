#include "star_agent.hpp"
#include "helpers/simple_compare.hpp"
#include "search_replace.hpp" 

using namespace SR;

// NOTE this is a DecidedCompare() not DecidedCompareImpl() so some of the AgentCommon 
// stuff has to be done explicitly in here.
bool StarAgent::DecidedCompare( const TreePtrInterface &x,
                                bool can_key,
                                Conjecture &conj )
{
    INDENT("*");

    // Coupling restriction check
    if( TreePtr<Node> keynode = GetCoupled() )
    {
        SimpleCompare sc;
        if( sc( TreePtr<Node>(x), keynode ) == false )
            return false;
    }
            
    // this is an abnormal context (which of the program nodes
    // in the range should key the pattern?) so just wave keying
    // pass right on through.
    if( can_key )
    {
        DoKey( TreePtr<Node>(x) );   
        return true;
    }
                
    ContainerInterface *xc = dynamic_cast<ContainerInterface *>(x.get());
    ASSERT(xc)("Nodes passed to StarAgent::DecidedCompare() must implement ContainerInterface, since * matches multiple things");
    
    links.clear();    

    if( TreePtr<Node> p = GetRestriction() )
    {
        TRACE("MatchRange pattern\n");
        // Apply pattern restriction - will be at least as strict as pre-restriction
        FOREACH( const TreePtrInterface &xe, *xc )
        {
            // NOTE only getting away with this because xc is still in scope when DecidedCompareLinks() is called
            // Resolve via a keep-alive in the link 
            RememberAbnormalLink( AsAgent(p), xe ); 
        }
        ASSERT(links.abnormal.size()==xc->size())("%d %d\n", links.abnormal.size(), xc->size());
    }
    else
    {
        TRACE("MatchRange pre-res\n");
        // No pattern, so just use own pre-restriction
        FOREACH( TreePtr<Node> xe, *xc )
        {
            if( !IsLocalMatch( xe.get()) )
                return false;
        }
    }     
    TRACE("done\n");
    return DecidedCompareLinks( can_key, conj ); 
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


