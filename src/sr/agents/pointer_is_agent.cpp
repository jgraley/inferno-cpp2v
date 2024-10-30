#include "pointer_is_agent.hpp"
#include "../search_replace.hpp"
#include "agent.hpp"
#include "../scr_engine.hpp"
#include "link.hpp"
#include "standard_agent.hpp"
#include "db/x_tree_database.hpp"

using namespace SR;

shared_ptr<PatternQuery> PointerIsAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
	pq->RegisterNormalLink( PatternLink(this, GetPointer()) );
    return pq;
}


TeleportAgent::QueryReturnType PointerIsAgent::RunTeleportQuery( const XTreeDatabase *db, Dependencies *deps, XLink stimulus_xlink ) const
{
	// Report dependency on parent node
	if( deps )
	{
		TreePtr<Node> parent_node = db->GetRow(stimulus_xlink).parent_node;
		if( parent_node )
		{	
			// If no parent node, there's no dep to declare, assuming root xlink
			// pointer type cannot change. Might be better to refuse the whole teleport, TBD.
			set<XLink> parent_xlinks = db->GetNodeRow(parent_node).parents;
			ASSERT( parent_xlinks.size() == 1 ); // parent_node has children so it should only have one parent (rule #217)
			
			deps->AddDep( OnlyElementOf(parent_xlinks).GetTreePtrInterface() );			
		}
	}
	
    // Get the pointer that points to us - now from the keyer x link
    const TreePtrInterface *px = stimulus_xlink.GetTreePtrInterface();
	ASSERT(px);     
	
	// Make an archetypical node matching the pointer's type
    TreePtr<Node> tnode = px->MakeValueArchetype();
    
    // Package up to indicate we don't have a parent for the new node
	return make_pair( XLink(), tnode );
}


Graphable::Block PointerIsAgent::GetGraphBlockInfo() const
{
	// The PointerIs node appears as a slightly flattened pentagon.
    Block block;
	block.bold = true;
	block.title = "PointerIs"; 
	block.shape = "house";
    block.block_type = Graphable::NODE_SHAPED;
    block.node = GetPatternPtr();
    auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(GetPointer()->get()),
              list<string>{},
              list<string>{},
              phase,
              GetPointer() );
    block.sub_blocks = { { "pointer", 
                           "", 
                           true,
                           { link } } };
    return block;
}


int PointerIsAgent::GetExtenderChannelOrdinal() const
{
	return 2;
}
