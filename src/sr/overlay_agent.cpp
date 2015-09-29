#include "overlay_agent.hpp" 
#include "green_grass_agent.hpp"
#include "helpers/simple_compare.hpp"
#include "search_replace.hpp" 

using namespace SR;

void OverlayAgent::PatternQueryImpl() const
{
	RememberLink( false, AsAgent(GetThrough()) );
}


bool OverlayAgent::DecidedQueryImpl( const TreePtrInterface &x ) const
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
    TRACE(*this)(" transferring key from ")(*AsAgent(GetThrough()))(" to ")(AsAgent(GetOverlay()));
    AsAgent(GetOverlay())->TrackingKey(AsAgent(GetThrough()));
    TRACE("Overlay node through=")(*(GetThrough()))(" overlay=")(*(GetOverlay()))("\n");
    return AsAgent(GetOverlay())->BuildReplace();
}


void OverlayAgent::GetGraphAppearance( bool *bold, string *text, string *shape ) const
{
	// The Overlay node is shown as a small triangle, with the through link on the right and the overlay link
	// coming out of the bottom.
	*bold = true;
	*shape = "triangle";
	*text = string(""); 
}

