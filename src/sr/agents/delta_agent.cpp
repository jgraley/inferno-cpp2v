#include "delta_agent.hpp" 
#include "green_grass_agent.hpp"
#include "../search_replace.hpp" 
#include "link.hpp"
#include "coupling.hpp"

using namespace SR;

shared_ptr<PatternQuery> DeltaAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>(this);
	pq->RegisterNormalLink( PatternLink(this, GetThrough()) );
    return pq;
}

Graphable::Block DeltaAgent::GetGraphBlockInfo( const LinkNamingFunction &lnf,
                                                const NonTrivialPreRestrictionFunction &ntprf ) const
{
    list<SubBlock> sub_blocks;
    auto link_through = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(GetThrough()->get()), 
              list<string>{},
              list<string>{PatternLink(this, GetThrough()).GetShortName()},
              IN_COMPARE_ONLY,
              SpecialBase::IsNonTrivialPreRestriction(GetThrough()) );
    auto link_overlay = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(GetOverlay()->get()), 
              list<string>{},
              list<string>{PatternLink(this, GetOverlay()).GetShortName()},
              IN_REPLACE_ONLY,
              SpecialBase::IsNonTrivialPreRestriction(GetOverlay()) );
    sub_blocks.push_back( { "overlay", 
                            "",
                            true,
                            { link_through, link_overlay } } );
    return { false, "Delta", "Δ", "triangle", NODE, sub_blocks };
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


void DeltaAgent::StartKeyForOverlay( map<PatternLink, PatternLink> &overlay_plinks )
{
    ASSERT( *GetOverlay() );          
    PatternLink overlay_plink(this, GetOverlay());
    PatternLink through_plink(this, GetThrough());
    
    // Key as many nodes as possible on the replace side. Note: the "keyer link"
    // is always the link coming from traversal of the subtree under GetOverlay()
    overlay_plink.GetChildAgent()->KeyForOverlay(overlay_plinks, overlay_plink, through_plink);
}


TreePtr<Node> DeltaAgent::BuildReplaceImpl( PatternLink me_plink, 
                                            TreePtr<Node> under_node ) 
{
    INDENT("O");    

    PatternLink overlay_plink(this, GetOverlay());
    return overlay_plink.GetChildAgent()->BuildReplace(overlay_plink);
}
