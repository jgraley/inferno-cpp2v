#include "overlay_agent.hpp" 
#include "green_grass_agent.hpp"
#include "../search_replace.hpp" 
#include "link.hpp"

using namespace SR;

shared_ptr<PatternQuery> OverlayAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>(this);
	pq->RegisterNormalLink( PatternLink(this, GetThrough()) );
    return pq;
}


void OverlayAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                        XLink x ) const
{
    query.Reset();
    
    // Check pre-restriction
    CheckLocalMatch(x.GetChildX().get());

    query.RegisterNormalLink( PatternLink(this, GetThrough()), x ); // Link into X
}


list<Graphable::SubBlock> OverlayAgent::GetGraphBlockInfo() const
{
    list<SubBlock> sub_blocks;
    sub_blocks.push_back( {"through", (TreePtr<Node>)*GetThrough(), "", PatternLink(this, GetThrough()).GetName() } );
    sub_blocks.push_back( {"overlay", (TreePtr<Node>)*GetOverlay(), "style=\"dashed\"\n", PatternLink(this, GetOverlay()).GetName() } );
    return sub_blocks;
}


shared_ptr<ContainerInterface> OverlayAgent::GetVisibleChildren( Path v ) const
{	
	shared_ptr< Sequence<Node> > seq( new Sequence<Node> );
    switch(v)
    {
    case COMPARE_PATH:
        seq->push_back( *GetThrough() );
        break;
    case REPLACE_PATH:
        seq->push_back( *GetOverlay() );
        break;
    }
	return seq;
}


TreePtr<Node> OverlayAgent::BuildReplaceImpl( CouplingKey keylink ) 
{
    INDENT("O");    
    ASSERT( *GetOverlay() );          
    // Key as many nodes as possible on the replace side
    TRACE(*this)(" transferring key from ")(*AsAgent((TreePtr<Node>)*GetThrough()))(" to ")(AsAgent((TreePtr<Node>)*GetOverlay()));
    AsAgent((TreePtr<Node>)*GetOverlay())->TrackingKey(AsAgent((TreePtr<Node>)*GetThrough()));
    TRACE("Overlay node through=")(*(*GetThrough()))(" overlay=")(*(*GetOverlay()))("\n");
    return AsAgent((TreePtr<Node>)*GetOverlay())->BuildReplace();
}
