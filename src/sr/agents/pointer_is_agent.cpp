#include "pointer_is_agent.hpp"
#include "../search_replace.hpp"
#include "agent.hpp"
#include "../scr_engine.hpp"
#include "link.hpp"
#include "standard_agent.hpp"

using namespace SR;

shared_ptr<PatternQuery> PointerIsAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>(this);
	pq->RegisterNormalLink( PatternLink(this, GetPointer()) );
    return pq;
}


LocatedLink PointerIsAgent::RunTeleportQuery( XLink keyer_xlink ) const
{
    // Get the pointer that points to us - now from the keyer x link
    const TreePtrInterface *px = keyer_xlink.GetXPtr();
	ASSERT(px);     
	
	// Make an archetypical node matching the pointer's type
	TreePtr<Node> ptr_arch = px->MakeValueArchetype();

	// We need an XLink to the archetype
	XLink tp_xlink = XLink::CreateDistinct(ptr_arch); // Cache will un-distinct
	
	// Return a located link
	return { { PatternLink(this, GetPointer()), tp_xlink } };
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
