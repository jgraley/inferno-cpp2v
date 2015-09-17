#include "overlay_agent.hpp" 
#include "green_grass_agent.hpp"
#include "helpers/simple_compare.hpp"
#include "search_replace.hpp" 

using namespace SR;

deque<Agent *> OverlayAgent::PatternQuery() const
{
	deque<Agent *> child_agents;
	child_agents.push_back( AsAgent(GetThrough()) );
	// Not overlay as that is replace-side only
	return child_agents;
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
    AsAgent(GetOverlay())->TrackingKey(AsAgent(GetThrough()));
    TRACE("Overlay node through=")(*(GetThrough()))(" overlay=")(*(GetOverlay()))("\n");
    return AsAgent(GetOverlay())->BuildReplace();
}


