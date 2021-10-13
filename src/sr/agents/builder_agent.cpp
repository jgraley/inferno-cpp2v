#include "builder_agent.hpp"
#include "coupling.hpp"
#include "scr_engine.hpp"

using namespace SR;

TreePtr<Node> BuilderAgent::BuildReplaceImpl( TreePtr<Node> under_node ) 
{
    INDENT("%");
    if( !under_node )
    {
        // Call the soft pattern impl 
        under_node = BuildNewSubtree();
        
        CouplingKey key(XLink::CreateDistinct( under_node ), KEY_PRODUCER_7 );
        master_scr_engine->SetReplaceKey( this, key );
    }
    
    // Note that the keylink could have been set via coupling - but still not
    // likely to do anything sensible, so explicitly check
    return DuplicateSubtree(under_node);   
}
