#include "embedded_scr_agent.hpp"
#include "../search_replace.hpp" 
#include "../scr_engine.hpp"
#include "../db/duplicate.hpp"
#include "link.hpp"

using namespace SR;

EmbeddedSCRAgent::EmbeddedSCRAgent( TreePtr<Node> sp, TreePtr<Node> rp, bool is_search_ ) :
    search_pattern( sp ),
    replace_pattern( rp ),
    is_search( is_search_ )
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
    // Make embedded engines "invisible" to Delta key propagation (i.e. Autolocated see #342)
    PatternLink through_plink(this, GetThrough());
    through_plink.GetChildAgent()->PlanOverlay(through_plink, under_plink);   
}


Agent::ReplacePatchPtr EmbeddedSCRAgent::GenReplaceLayoutImpl( const ReplaceKit &kit, 
                                                               PatternLink me_plink, 
                                                               XLink key_xlink )
{   
    // Use auto algorithm but add marker
    Agent::ReplacePatchPtr patch = AutolocatingAgent::GenReplaceLayoutImpl(kit, me_plink, key_xlink);

    // Inform the update mechanism that, once it's done duplicating 
    // nodes etc, it should mark this position for this embedded agent's origin.
    patch->AddOriginators( { me_plink } );

    return patch;
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


void EmbeddedSCRAgent::SetAssign( TreePtr<Node> embedded_origin ) const
{
    my_scr_engine->MarkOriginForEmbedded( this, embedded_origin );
}


TreePtr<Node> EmbeddedSCRAgent::GetEmbeddedSearchPattern() const 
{ 
    return search_pattern; 
}


TreePtr<Node> EmbeddedSCRAgent::GetEmbeddedReplacePattern() const 
{ 
    return replace_pattern; 
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

