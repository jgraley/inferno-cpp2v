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


map<PatternLink, XLink> PointerIsAgent::RunTeleportQuery( XLink base_xlink ) const
{
    TreePtr<Node> context = master_scr_engine->GetOverallMaster()->GetContext();
    if( base_xlink.GetChildX() == context )
    {
        // Imagine that the root is pointed to by a TreePtr<Node>
        // (in this case wit.GetNodePointerInParent() would return NULL)
        TreePtr<Node> node( new Node );
        XLink tp_xlink = XLink::CreateDistinct(node);	// Cache will un-distinct        
        return { { PatternLink(this, GetPointer()), tp_xlink } };
    }
    
    // Do a walk over context (the whole x tree)
    bool found_one_already = false;
    Walk e( context ); 
    for( Walk::iterator wit=e.begin(); wit!=e.end(); ++wit )
    {
        if( *wit == base_xlink.GetChildX() ) // found ourself TODO use find()
        {            
            if(found_one_already)
                throw Mismatch(); // X has multiple parents - ambiguous, so don't match
            found_one_already = true;
                
            // Get the pointer that points to us
            const TreePtrInterface *px = wit.GetNodePointerInParent();    
            ASSERT(px);     
            // Make an architypical node matching the pointer's type
            TreePtr<Node> ptr_arch = px->MakeValueArchitype();

            // Stick that in your pipe + smoke it
            XLink tp_xlink = XLink::CreateDistinct(ptr_arch); // Cache will un-distinct
            return { { PatternLink(this, GetPointer()), tp_xlink } };
        }
    }
    
    if(!found_one_already)
    {
        // If there's no parent we must be at the root of the x tree,
        // so simulate a link that allows anything (because in fact
        // you can replace the root node with anything).
        TreePtr<Node> node( new Node );
        XLink tp_xlink = XLink::CreateDistinct(node);	// Cache will un-distinct
        return { { PatternLink(this, GetPointer()), tp_xlink } };
    }
    ASSERTFAIL("Failed to generate a link\n");
}


Graphable::Block PointerIsAgent::GetGraphBlockInfo( const LinkNamingFunction &lnf ) const
{
	// The PointerIs node appears as a slightly flattened pentagon.
    Block block;
	block.bold = true;
	block.title = "PointerIs"; 
	block.shape = "house";
    block.block_type = Graphable::NODE;
    block.sub_blocks = { { "pointer", 
                           "", 
                           true,
                           { { (TreePtr<Node>)*GetPointer(), 
                               GetPointer(),
                               THROUGH, 
                               {},
                               {PatternLink(this, GetPointer()).GetShortName()} } } } };
    return block;
}
