#include "slave_agent.hpp"
#include "../search_replace.hpp" 
#include "../scr_engine.hpp"
#include "link.hpp"
#include "coupling.hpp"

using namespace SR;

SlaveAgent::SlaveAgent( TreePtr<Node> sp, TreePtr<Node> rp, bool is_search_ ) :
    is_search( is_search_ ),
    my_scr_engine( nullptr ),
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


void SlaveAgent::ConfigureMyEngine( SCREngine *my_scr_engine_ )
{
    my_scr_engine = my_scr_engine_;
}


void SlaveAgent::KeyForOverlay( map<PatternLink, PatternLink> &overlay_plinks, PatternLink me_plink, PatternLink under_plink )
{
    INDENT("l");
    ASSERT( me_plink.GetChildAgent() == this );

    overlay_plinks[me_plink] = under_plink;
    
    // Make slaves "invisible" to Delta key propagation (i.e. Colocated see #342)
    PatternLink through_plink(this, GetThrough());
    through_plink.GetChildAgent()->KeyForOverlay(overlay_plinks, through_plink, under_plink);   
}


TreePtr<Node> SlaveAgent::BuildReplaceImpl( PatternLink me_plink, 
                                            TreePtr<Node> under_node )
{
    INDENT("l");
    ASSERT( *GetThrough() );   
    
    // Continue current replace operation by following the "through" pointer
    PatternLink through_plink(this, GetThrough());
    TreePtr<Node> my_through_subtree = through_plink.GetChildAgent()->BuildReplace(through_plink);
    ASSERT( my_through_subtree );
    
    // And then recurse into slaves
    master_scr_engine->RecurseInto( my_scr_engine, &my_through_subtree );   
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


Graphable::Block SlaveAgent::GetGraphBlockInfo( const LinkNamingFunction &lnf,
                                     const NonTrivialPreRestrictionFunction &ntprf ) const
{
    list<SubBlock> sub_blocks;
    // Actually much simpler in graph trace mode - just show the root node and plink
    auto compare_link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(search_pattern.get()),
                                                      list<string>{},
                                                      list<string>{""},
                                                      IN_COMPARE_AND_REPLACE,
                                                      SpecialBase::IsNonTrivialPreRestriction(&search_pattern) );                                  
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
                                                     SpecialBase::IsNonTrivialPreRestriction(&replace_pattern) );                                  
    
        sub_blocks.push_back( { "replace",
                                "",
                                true,
                                { replace_link } } );
    }
    Block block = { false, GetName(), "", "", CONTROL, sub_blocks };
   
    auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(GetThrough()->get()), 
              list<string>{},
              list<string>{PatternLink(this, GetThrough()).GetShortName()},
              phase,
              SpecialBase::IsNonTrivialPreRestriction(GetThrough()) );
    block.sub_blocks.push_front( { "through", 
                                   "",
                                   true,
                                   { link } } );
    return block;
}

