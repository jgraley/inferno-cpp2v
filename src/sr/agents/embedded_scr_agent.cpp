#include "embedded_scr_agent.hpp"
#include "../search_replace.hpp" 
#include "../scr_engine.hpp"
#include "../duplicate.hpp"
#include "link.hpp"
#include "up/tree_update.hpp"

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


Agent::CommandPtr EmbeddedSCRAgent::GenerateCommandImpl( const ReplaceKit &kit, 
                                                         PatternLink me_plink, 
                                                         XLink key_xlink )
{
    auto commands = make_unique<CommandSequence>();
    
    PatternLink through_plink(this, GetThrough());
    commands->Add( through_plink.GetChildAgent()->GenerateCommand(kit, through_plink) );    
    
    // Indicates that the embedded engine should act at the base
    // of whatever's at the top of the stack, which should be the 
    // though subtree due to the previous command.
    commands->Add( make_unique<MarkBaseForEmbeddedCommand>( this ) );  
    
    return commands;
}                                         


list<PatternLink> EmbeddedSCRAgent::GetVisibleChildren( Path v ) const
{
	// it's an embedded engines, so set up a container containing only "through", not "compare" or "replace"
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

