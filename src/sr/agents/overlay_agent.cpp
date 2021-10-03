#include "overlay_agent.hpp" 
#include "green_grass_agent.hpp"
#include "../search_replace.hpp" 
#include "link.hpp"
#include "coupling.hpp"

using namespace SR;

shared_ptr<PatternQuery> OverlayAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>(this);
	pq->RegisterNormalLink( PatternLink(this, GetThrough()) );
    return pq;
}

Graphable::Block OverlayAgent::GetGraphBlockInfo( const LinkNamingFunction &lnf,
                                     const NonTrivialPreRestrictionFunction &ntprf ) const
{
    list<SubBlock> sub_blocks;
    auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(GetThrough()->get()), 
              list<string>{},
              list<string>{PatternLink(this, GetThrough()).GetShortName()},
              IN_COMPARE_ONLY,
              SpecialBase::IsNonTrivialPreRestriction(GetThrough()) );
    sub_blocks.push_back( { "through", 
                            "",
                            false,
                            { link } } );
    link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(GetOverlay()->get()), 
              list<string>{},
              list<string>{PatternLink(this, GetOverlay()).GetShortName()},
              IN_REPLACE_ONLY,
              SpecialBase::IsNonTrivialPreRestriction(GetOverlay()) );
    sub_blocks.push_back( { "overlay", 
                            "",
                            false,
                            { link } } );
    return { false, GetName(), "", "", CONTROL, sub_blocks };
}


list<PatternLink> OverlayAgent::GetVisibleChildren( Path v ) const
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
