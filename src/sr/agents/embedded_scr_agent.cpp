#include "embedded_scr_agent.hpp"
#include "../search_replace.hpp" 
#include "../scr_engine.hpp"
#include "link.hpp"

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
    // Make slaves "invisible" to Delta key propagation (i.e. Colocated see #342)
    PatternLink through_plink(this, GetThrough());
    through_plink.GetChildAgent()->PlanOverlay(through_plink, under_plink);   
}


TreePtr<Node> EmbeddedSCRAgent::BuildReplaceImpl( PatternLink me_plink )
{
    INDENT("l");
    ASSERT( *GetThrough() );   
    
    // Continue current replace operation by following the "through" pointer
    PatternLink through_plink(this, GetThrough());
    TreePtr<Node> my_through_subtree = through_plink.GetChildAgent()->BuildReplace(through_plink);
    ASSERT( my_through_subtree );
    
    // And then recurse into slaves
    master_scr_engine->RequestEmbeddedAction( this, my_through_subtree );   
    ASSERT( my_through_subtree );
    
    return my_through_subtree;
}


list<PatternLink> EmbeddedSCRAgent::GetVisibleChildren( Path v ) const
{
	// it's a slave, so set up a container containing only "through", not "compare" or "replace"
	list<PatternLink> plinks;
	plinks.push_back( PatternLink(this, GetThrough()) );
	return plinks;
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
