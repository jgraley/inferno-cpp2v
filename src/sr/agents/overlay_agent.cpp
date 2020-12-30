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


Graphable::Block OverlayAgent::GetGraphBlockInfo() const
{
    list<SubBlock> sub_blocks;
    sub_blocks.push_back( { "through", 
                            "",
                            { { (TreePtr<Node>)*GetThrough(), 
                                GetThrough(),
                                SOLID, 
                                {},
                                {PatternLink(this, GetThrough()).GetShortName()} } } } );
    sub_blocks.push_back( { "overlay", 
                            "",
                            { { (TreePtr<Node>)*GetOverlay(), 
                                GetOverlay(),
                                DASHED, 
                                {},
                                {PatternLink(this, GetOverlay()).GetShortName()} } } } );
    return { false, GetName(), "", ENGINE, sub_blocks };
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


TreePtr<Node> OverlayAgent::BuildReplaceImpl() 
{
    INDENT("O");    
    ASSERT( *GetOverlay() );          
    Agent *overlay_agent = AsAgent((TreePtr<Node>)*GetOverlay());
    Agent *through_agent = AsAgent((TreePtr<Node>)*GetThrough());
    
    // Key as many nodes as possible on the replace side
    TRACE(*this)(" transferring key from ")(*through_agent)(" to ")(*overlay_agent);
    overlay_agent->KeyForOverlay(through_agent);
    return overlay_agent->BuildReplace();
}
