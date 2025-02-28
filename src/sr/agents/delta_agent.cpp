#include "delta_agent.hpp" 
#include "green_grass_agent.hpp"
#include "../search_replace.hpp" 
#include "link.hpp"

using namespace SR;

shared_ptr<PatternQuery> DeltaAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
	pq->RegisterNormalLink( PatternLink(this, GetThrough()) );
    return pq;
}

Graphable::Block DeltaAgent::GetGraphBlockInfo() const
{
    list<SubBlock> sub_blocks;
    auto link_through = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(GetThrough()->get()), 
              list<string>{},
              list<string>{},
              IN_COMPARE_ONLY,
              GetThrough() );
    sub_blocks.push_back( { "through", 
                            "",
                            true,
                            { link_through } } );
    auto link_overlay = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(GetOverlay()->get()), 
              list<string>{},
              list<string>{},
              IN_REPLACE_ONLY,
              GetOverlay() );
    sub_blocks.push_back( { "overlay", 
                            "",
                            true,
                            { link_overlay } } );
    return { false, "Delta", "Δ", "triangle", NODE_SHAPED, GetPatternPtr(), sub_blocks };
}


list<PatternLink> DeltaAgent::GetVisibleChildren( Path v ) const
{	
	list<PatternLink> plinks;
    switch(v)
    {
    case COMPARE_PATH:
        plinks.push_back( PatternLink(this, GetThrough()) );
        break;
    case REPLACE_PATH:
        plinks.push_back( PatternLink(this, GetOverlay()) );
        break;
    }
	return plinks;
}


void DeltaAgent::StartPlanOverlay()
{
    ASSERT( *GetOverlay() );          
    PatternLink overlay_plink(this, GetOverlay());
    PatternLink through_plink(this, GetThrough());
    
    // Key as many nodes as possible on the replace side. Note: the "keyer link"
    // is always the link coming from traversal of the subtree under GetOverlay()
    overlay_plink.GetChildAgent()->PlanOverlay(overlay_plink, through_plink);
}


Agent::ReplacePatchPtr DeltaAgent::GenReplaceLayoutImpl( const ReplaceKit &kit, 
                                                   PatternLink me_plink, 
                                                   XLink key_xlink )
{
	// Recurse through the Overlay branch
    PatternLink overlay_plink(this, GetOverlay());
    return overlay_plink.GetChildAgent()->GenReplaceLayout(kit, overlay_plink);    
}                                         
