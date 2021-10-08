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


void SlaveAgent::KeyForOverlay( Agent *from )
{
    INDENT("l");
    // Make slaves "invisible" to Overlay key propagation
    SetKey( from->GetKey() );
    AsAgent((TreePtr<Node>)*GetThrough())->KeyForOverlay(from);   
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


void SlaveAgent::ConfigureMyEngine( SCREngine *my_scr_engine_ )
{
    my_scr_engine = my_scr_engine_;
}


void SlaveAgent::SetMasterCouplingKeys( const CouplingKeysMap &keys )
{
	master_keys = keys;
}


TreePtr<Node> SlaveAgent::BuildReplaceImpl( TreePtr<Node> keynode )
{
    INDENT("l");
    ASSERT( *GetThrough() );   
    
    // Continue current replace operation by following the "through" pointer
    TreePtr<Node> my_root_xnode = AsAgent((TreePtr<Node>)*GetThrough())->BuildReplace();
    ASSERT( my_root_xnode );
    
    // And then recurse into slaves
    master_scr_engine->RecurseInto( my_scr_engine, &my_root_xnode );   
    ASSERT( my_root_xnode );
    
    return my_root_xnode;
}


list<PatternLink> SlaveAgent::GetVisibleChildren( Path v ) const
{
	// it's a slave, so set up a container containing only "through", not "compare" or "replace"
	list<PatternLink> plinks;
	plinks.push_back( PatternLink(this, GetThrough()) );
	return plinks;
}
