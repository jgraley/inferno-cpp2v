#include "slave_agent.hpp"
#include "../search_replace.hpp" 
#include "../scr_engine.hpp"
#include "link.hpp"

using namespace SR;

SlaveAgent::SlaveAgent( TreePtr<Node> sp, TreePtr<Node> rp, bool is_search_ ) :
    is_search( is_search_ ),
    search_pattern( sp ),
    replace_pattern( rp )
{
}


shared_ptr<PatternQuery> SlaveAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>(this);
	pq->RegisterNormalLink( PatternLink(this, GetThrough()) );
    return pq;
}


void SlaveAgent::MaybeChildrenPlanOverlay( PatternLink me_plink, 
                                  PatternLink under_plink )
{    
    // Make slaves "invisible" to Delta key propagation (i.e. Colocated see #342)
    PatternLink through_plink(this, GetThrough());
    through_plink.GetChildAgent()->PlanOverlay(through_plink, under_plink);   
}


TreePtr<Node> SlaveAgent::BuildReplaceImpl( PatternLink me_plink, 
                                            TreePtr<Node> key_node )
{
    INDENT("l");
    ASSERT( *GetThrough() );   
    
    // Continue current replace operation by following the "through" pointer
    PatternLink through_plink(this, GetThrough());
    TreePtr<Node> my_through_subtree = through_plink.GetChildAgent()->BuildReplace(through_plink);
    ASSERT( my_through_subtree );
    
    // And then recurse into slaves
    master_scr_engine->RequestSlaveAction( this, my_through_subtree );   
    ASSERT( my_through_subtree );
    
    return my_through_subtree;
}


list<PatternLink> SlaveAgent::GetVisibleChildren( Path v ) const
{
	// it's a slave, so set up a container containing only "through", not "compare" or "replace"
	list<PatternLink> plinks;
	plinks.push_back( PatternLink(this, GetThrough()) );
	return plinks;
}


Graphable::Block SlaveAgent::GetGraphBlockInfo() const
{
    list<SubBlock> sub_blocks;
    // Actually much simpler in graph trace mode - just show the root node and plink
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

