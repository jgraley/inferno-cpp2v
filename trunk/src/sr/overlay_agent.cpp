#include "overlay_agent.hpp" 
#include "green_grass_agent.hpp"
#include "helpers/simple_compare.hpp"
#include "search_replace.hpp" 

using namespace SR;

bool OverlayAgent::DecidedQueryImpl( const TreePtrInterface &x,
                                     bool can_key )
{
    // Check pre-restriction
    if( !IsLocalMatch(x.get()) )        
        return false;

    RememberLink( false, AsAgent(GetThrough()), x );
    return true;
}


TreePtr<Node> OverlayAgent::BuildReplaceImpl( TreePtr<Node> keynode ) 
{
    INDENT("O");    
    ASSERT( GetOverlay() );          
    // Key as many nodes as possible on the replace side
    AsAgent(GetOverlay())->TrackingKey(AsAgent(GetThrough()));
    TRACE("Overlay node through=")(*(GetThrough()))(" overlay=")(*(GetOverlay()))("\n");
    return AsAgent(GetOverlay())->BuildReplace();
}


