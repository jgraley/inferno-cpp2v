#include "embedded_scr_agent.hpp"
#include "../search_replace.hpp" 
#include "../scr_engine.hpp"
#include "../db/duplicate.hpp"
#include "link.hpp"
#include "lang/render.hpp"

using namespace VN;

EmbeddedSCRAgent::EmbeddedSCRAgent( TreePtr<Node> sp, TreePtr<Node> rp, bool is_search_ ) :
    search_pattern( sp ),
    replace_pattern( rp ),
    is_search( is_search_ )
{
}


shared_ptr<PatternQuery> EmbeddedSCRAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
    pq->RegisterNormalLink( PatternLink(GetThrough()) );
    return pq;
}


void EmbeddedSCRAgent::MaybeChildrenPlanOverlay( SCREngine *acting_engine,
												 PatternLink me_plink, 
                                                 PatternLink bottom_layer_plink )
{    
	(void)me_plink;
    // Make embedded engines "invisible" to Delta key propagation (i.e. Autolocated see #342)
    PatternLink through_plink(GetThrough());
    through_plink.GetChildAgent()->PlanOverlay(acting_engine, through_plink, bottom_layer_plink);   
}                                      


list<PatternLink> EmbeddedSCRAgent::GetVisibleChildren( Path v ) const
{
	(void)v;
    // it's an embedded engines, so set up a container containing only "through", not "compare" or "replace"
    list<PatternLink> plinks;
    plinks.push_back( PatternLink(GetThrough()) );
    return plinks;
}


bool EmbeddedSCRAgent::IsSearch() const
{ 
    return is_search; 
}


TreePtr<Node> EmbeddedSCRAgent::GetEmbeddedSearchPattern() const 
{ 
    return search_pattern; 
}


TreePtr<Node> EmbeddedSCRAgent::GetEmbeddedReplacePattern() const 
{ 
    return replace_pattern; 
}


Syntax::Production EmbeddedSCRAgent::GetAgentProduction() const
{
	return Syntax::Production::VN_SEP;
}


string EmbeddedSCRAgent::GetRender( const RenderKit &kit, string prefix, Syntax::Production surround_prod ) const
{
	(void)surround_prod;
	if( search_pattern==replace_pattern && !is_search )	
		return prefix + "⦑" + // TODO implement GetMyProduction and automatically boot to VN_SEP when required
			   kit.render( string(), (TreePtr<Node>)(*GetThrough()), Syntax::Production::VN_SEP ) + 
			   "︙\n" +
			   kit.render( "꩜", search_pattern, Syntax::BoostPrecedence( Syntax::Production::VN_SEP ) ) + // Left-associative
 			   "⦒";
	else
		return "😦"; // Should have done pattern transformations to get rid of this
}    
    
    


Graphable::NodeBlock EmbeddedSCRAgent::GetGraphBlockInfo() const
{
    list<ItemBlock> item_blocks;
    // Actually much simpler in graph trace mode - just show the base node and plink
    auto compare_link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(search_pattern.get()),
                                                      list<string>{},
                                                      list<string>{""},
                                                      IN_COMPARE_AND_REPLACE,
                                                      &search_pattern );                                  
    item_blocks.push_back( { "search/compare", 
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
    
        item_blocks.push_back( { "replace",
                                "",
                                true,
                                { replace_link } } );
    }
   
    auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(GetThrough()->get()), 
              list<string>{},
              list<string>{},
              phase,
              GetThrough() );
    item_blocks.push_front( { "through", 
                             "",
                             true,
                             { link } } );
    NodeBlock block = { false, GetName(), "", "", CONTROL, GetPatternPtr(), item_blocks };
    return block;
}

