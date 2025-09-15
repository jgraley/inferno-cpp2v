#include "delta_agent.hpp" 
#include "green_grass_agent.hpp"
#include "../search_replace.hpp" 
#include "link.hpp"

using namespace VN;

shared_ptr<PatternQuery> DeltaAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
    pq->RegisterNormalLink( PatternLink(GetThrough()) );
    return pq;
}


Graphable::NodeBlock DeltaAgent::GetGraphBlockInfo() const
{
    list<ItemBlock> item_blocks;
    auto link_through = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(GetThrough()->get()), 
              list<string>{},
              list<string>{},
              IN_COMPARE_ONLY,
              GetThrough() );
    item_blocks.push_back( { "through", 
                            "",
                            true,
                            { link_through } } );
    auto link_overlay = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(GetOverlay()->get()), 
              list<string>{},
              list<string>{},
              IN_REPLACE_ONLY,
              GetOverlay() );
    item_blocks.push_back( { "overlay", 
                            "",
                            true,
                            { link_overlay } } );
    return { false, "Delta", "Δ", "triangle", NODE_SHAPED, GetPatternPtr(), item_blocks };
}


list<PatternLink> DeltaAgent::GetVisibleChildren( Path v ) const
{    
    ASSERT( *GetOverlay() );          
    ASSERT( *GetThrough() );          
    
    list<PatternLink> plinks;
    switch(v)
    {
    case COMPARE_PATH:
        plinks.push_back( PatternLink(GetThrough()) );
        break;
    case REPLACE_PATH:
        plinks.push_back( PatternLink(GetOverlay()) );
        break;
    }
    return plinks;
}


void DeltaAgent::StartPlanOverlay(SCREngine *acting_engine)
{
    ASSERT( *GetOverlay() );          
    PatternLink overlay_plink(GetOverlay());
    PatternLink through_plink(GetThrough());
    
    overlay_plink.GetChildAgent()->PlanOverlay(acting_engine, overlay_plink, through_plink);
}


Agent::ReplacePatchPtr DeltaAgent::GenReplaceLayoutImpl( const ReplaceKit &kit, 
                                                         PatternLink me_plink, 
                                                         XLink key_xlink,
                                                         const SCREngine *acting_engine )
{
	(void)me_plink;
	(void)key_xlink;
	
    // Recurse through the Overlay branch
    PatternLink overlay_plink(GetOverlay());
    return overlay_plink.GetChildAgent()->GenReplaceLayout(kit, overlay_plink, acting_engine);    
}                                         
