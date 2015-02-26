#include "star_agent.hpp"
#include "helpers/simple_compare.hpp"
#include "search_replace.hpp" // TODO should not need

using namespace SR;

bool StarAgent::DecidedCompareImpl( const TreePtrInterface &x,
                                    bool can_key,
                                    Conjecture &conj )
{
    ASSERTFAIL("Can only use Star in a container"); 
}


bool StarAgent::CompareRange( ContainerInterface &range,
                              bool can_key,
                              Conjecture &conj )
{
    INDENT;
    // this is an abnormal context (which of the program nodes
    // in the range should key the pattern?) so just wave keying
    // pass right on through.
    if( can_key )
        return true;
                
    TreePtr<Node> p = GetPattern();
    if( p )
    {
        TRACE("MatchRange pattern\n");
        // Apply pattern restriction - will be at least as strict as pre-restriction
        FOREACH( TreePtr<Node> x, range )
        {
            bool r = AsAgent(p)->Compare( x, false, &conj ); 
            if( !r )
                return false;
        }
    }
    else
    {
        TRACE("MatchRange pre-res\n");
        // No pattern, so just use pre-restrictions
        FOREACH( TreePtr<Node> x, range )
        {
            if( !IsLocalMatch( x.get()) )
                return false;
        }
    }     
    TRACE("done\n");
    return true;   
}                       


TreePtr<Node> StarAgent::BuildReplaceImpl( TreePtr<Node> keynode ) 
{
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


