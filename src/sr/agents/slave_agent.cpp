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


void SlaveAgent::KeyForOverlay( Agent *from )
{
    INDENT("l");
    // Make slaves "invisible" to Overlay key propagation
    CouplingKey from_key = master_scr_engine->GetReplaceKey( from );
    master_scr_engine->SetReplaceKey( this, from_key );
    
    AsAgent((TreePtr<Node>)*GetThrough())->KeyForOverlay(from);   
}


TreePtr<Node> SlaveAgent::BuildReplaceImpl( TreePtr<Node> keynode )
{
    INDENT("l");
    ASSERT( *GetThrough() );   
    
    // Continue current replace operation by following the "through" pointer
    TreePtr<Node> my_through_subtree = AsAgent((TreePtr<Node>)*GetThrough())->BuildReplace();
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
	ASSERT( master_scr_engine )("Agent must before configured before graphing");

    Block block = my_scr_engine->GetGraphBlockInfo(lnf, ntprf);
    block.title = "Slave";
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


