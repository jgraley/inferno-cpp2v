#include "overlay_agent.hpp" 
#include "green_grass_agent.hpp"
#include "helpers/simple_compare.hpp"
#include "search_replace.hpp" 

using namespace SR;

bool OverlayAgent::DecidedCompareImpl( const TreePtrInterface &x,
                                       bool can_key )
{
    // Check pre-restriction
    if( !IsLocalMatch(x.get()) )        
        return false;

    RememberLink( false, AsAgent(GetThrough()), x );
    return true;
}


void OverlayAgent::KeyReplace()
{
    INDENT("O");
    // Give the overlay side a nudge in case it wants to key itself eg due to
    // Builder node. TODO avoid repeat calls to KeyReplace()    
    AsAgent(GetOverlay())->KeyReplace();
    
    // Now, if the overlay side did not key itself, key it per the through 
    // side key that will have been obtained during search. Thus, the immediate
    // children of Overlay appear as though coupled.

    // Key as many nodes as possible on the replace side
    AsAgent(GetOverlay())->TrackingKey(AsAgent(GetThrough()));
}


TreePtr<Node> OverlayAgent::BuildReplaceImpl( TreePtr<Node> keynode ) 
{
    INDENT("O");
    ASSERT( GetOverlay() );          
    TRACE("Overlay node through=")(*(GetThrough()))(" overlay=")(*(GetOverlay()))("\n");
    return AsAgent(GetOverlay())->BuildReplace();
}

