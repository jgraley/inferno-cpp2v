#include "embedded_scr_agent.hpp"
#include "../search_replace.hpp" 
#include "../scr_engine.hpp"
#include "../up/duplicate.hpp"
#include "link.hpp"
#include "up/commands.hpp"

using namespace SR;

EmbeddedSCRAgent::EmbeddedSCRAgent( TreePtr<Node> sp, TreePtr<Node> rp, bool is_search_ ) :
    is_search( is_search_ ),
    search_pattern( sp ),
    replace_pattern( rp )
{
}


shared_ptr<PatternQuery> EmbeddedSCRAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
	pq->RegisterNormalLink( PatternLink(this, GetThrough()) );
    return pq;
}


void EmbeddedSCRAgent::MaybeChildrenPlanOverlay( PatternLink me_plink, 
                                                 PatternLink under_plink )
{    
    // Make embedded engines "invisible" to Delta key propagation (i.e. Colocated see #342)
    PatternLink through_plink(this, GetThrough());
    through_plink.GetChildAgent()->PlanOverlay(through_plink, under_plink);   
}


Agent::ReplaceExprPtr EmbeddedSCRAgent::GenReplaceExprImpl( const ReplaceKit &kit, 
                                                         PatternLink me_plink, 
                                                         XLink key_xlink )
{   
	// Use colo algorithm but add marker
    Agent::ReplaceExprPtr child_command = ColocatedAgent::GenReplaceExprImpl(kit, me_plink, key_xlink);
    
    auto child_pzc = dynamic_cast<PopulateZoneOperator *>(child_command.get());
    ASSERT( child_pzc );
    
    // Inform the update mechanism that, once it's done duplicating 
    // nodes etc, it should mark this position for this embedded agent.
    child_pzc->AddEmbeddedMarker( this );

    return child_command;
}                                         


list<PatternLink> EmbeddedSCRAgent::GetVisibleChildren( Path v ) const
{
	// it's an embedded engines, so set up a container containing only "through", not "compare" or "replace"
	list<PatternLink> plinks;
	plinks.push_back( PatternLink(this, GetThrough()) );
	return plinks;
}


bool EmbeddedSCRAgent::IsSearch() const
{ 
	return is_search; 
}


void EmbeddedSCRAgent::MarkBaseForEmbedded( TreePtr<Node> embedded_through_subtree ) const
{
	my_scr_engine->MarkBaseForEmbedded( this, embedded_through_subtree );
}


TreePtr<Node> EmbeddedSCRAgent::GetSearchPattern() const 
{ 
	return search_pattern; 
}


TreePtr<Node> EmbeddedSCRAgent::GetReplacePattern() const 
{ 
	return replace_pattern; 
}


Graphable::Block EmbeddedSCRAgent::GetGraphBlockInfo() const
{
    list<SubBlock> sub_blocks;
    // Actually much simpler in graph trace mode - just show the base node and plink
    auto compare_link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(search_pattern.get()),
                                                      list<string>{},
                                                      list<string>{""},
                                                      IN_COMPARE_AND_REPLACE,
                                                      &search_pattern );                                  
    sub_blocks.push_back( { "search/compare", 
                            "",
                            true,
                            { compare_link } } );

    if( replace_pattern && replace_pattern != search_pattern )
    {
        auto replace_link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(replace_pattern.get()),
                                                     list<string>{},
                                                     list<string>{""},
                                                     IN_REPLACE_ONLY,
                                                     &replace_pattern );                                  
    
        sub_blocks.push_back( { "replace",
                                "",
                                true,
                                { replace_link } } );
    }
   
    auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(GetThrough()->get()), 
              list<string>{},
              list<string>{},
              phase,
              GetThrough() );
    sub_blocks.push_front( { "through", 
                             "",
                             true,
                             { link } } );
    Block block = { false, GetName(), "", "", CONTROL, GetPatternPtr(), sub_blocks };
    return block;
}

