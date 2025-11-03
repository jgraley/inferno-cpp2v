#include "pointer_is_agent.hpp"
#include "../search_replace.hpp"
#include "agent.hpp"
#include "../scr_engine.hpp"
#include "link.hpp"
#include "standard_agent.hpp"
#include "db/x_tree_database.hpp"
#include "lang/render.hpp"

using namespace VN;

shared_ptr<PatternQuery> PointerIsAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
    pq->RegisterNormalLink( PatternLink(GetPointer()) );
    return pq;
}


RelocatingAgent::RelocatingQueryResult PointerIsAgent::RunRelocatingQuery( const XTreeDatabase *db, XLink stimulus_xlink ) const
{
    // Report dependency on parent node
    Dependencies deps;
    TreePtr<Node> parent_node = db->GetRow(stimulus_xlink).parent_node;
    if( parent_node )
    {    
        // If no parent node, there's no dep to declare, assuming root xlink
        // pointer type cannot change. Might be better to refuse the query, TBD.
        set<XLink> parent_xlinks = db->GetNodeRow(parent_node).incoming_xlinks;
        ASSERT( parent_xlinks.size() == 1 ); // parent_node has children so it should be the sole parent (rule #217)
        
        deps.AddDep( SoloElementOf(parent_xlinks) );            
    }
    
    // Get the pointer that points to us - now from the keyer x link
    const TreePtrInterface *px = stimulus_xlink.GetTreePtrInterface();
    ASSERT(px);     
    
    // Make an archetypical node matching the pointer's type
    TreePtr<Node> induced_base_node = px->MakeValueArchetype();
    //FTRACE("stimulus_xlink: ")(stimulus_xlink)(", induced_base_node: ")(induced_base_node)("\n");
    
    // Major plot hole: even though there's a reasonable expectation that the pointer
    // is an intermediate type, it can still have children, for example Base has an access
    // and Loop has a body. We can't fill these in with wildcards because relocation
    // is meant to find/induce X tree nodes, and we can't leave them NULL. It might be
    // necessary to do "recursive plugging" in which we recursively fill in the gaps
    // with the pointed-to type until there are no more gaps. 
    if( !induced_base_node->Itemise().empty() )
		throw HasChildrenMismatch();
    
    // Package up to indicate we don't have a parent for the new node
    return RelocatingQueryResult( induced_base_node, deps );
}


int PointerIsAgent::GetExtenderChannelOrdinal() const
{
    return 2;
}


Syntax::Production PointerIsAgent::GetAgentProduction() const
{
	return Syntax::Production::PREFIX;
}


string PointerIsAgent::GetRender( const RenderKit &kit, Syntax::Production surround_prod ) const
{
	(void)surround_prod;
	return "⮎" + kit.render( (TreePtr<Node>)(*GetPointer()), Syntax::Production::PREFIX );
} 


Graphable::NodeBlock PointerIsAgent::GetGraphBlockInfo() const
{
    // The PointerIs node appears as a slightly flattened pentagon.
    NodeBlock block;
    block.bold = true;
    block.title = "PointerIs"; 
    block.symbol = "⮎";
    block.shape = "house";
    block.block_type = Graphable::NODE_SHAPED;
    block.node = GetPatternPtr();
    auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(GetPointer()->get()),
              list<string>{},
              list<string>{},
              phase,
              GetPointer() );
    block.item_blocks = { { "pointer", 
                           "", 
                           true,
                           { link } } };
    return block;
}
