#include "overlay_agent.hpp" 
#include "green_grass_agent.hpp"
#include "helpers/simple_compare.hpp"
#include "search_replace.hpp" 

using namespace SR;

shared_ptr<PatternQuery> OverlayAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
	pq->RegisterNormalLink( GetThrough() );
    return pq;
}


void OverlayAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                 const TreePtrInterface *px ) const
{
    query.Reset();
    
    // Check pre-restriction
    CheckLocalMatch(px->get());

    query.RegisterNormalLink( GetThrough(), px );
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
	// The Overlay node is shown as a small triangle, with the through block on the right and the overlay block
	// coming out of the bottom.
	*bold = true;
	*shape = "triangle";
	*text = string(""); 
}

