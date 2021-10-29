#include "builder_agent.hpp"
#include "coupling.hpp"
#include "scr_engine.hpp"

using namespace SR;

TreePtr<Node> BuilderAgent::BuildReplaceImpl( PatternLink me_plink, 
                                              TreePtr<Node> key_node ) 
{
    INDENT("%");
    if( !key_node )
    {
        // Call the soft pattern impl 
        key_node = BuildNewSubtree();
        
        LocatedLink link( me_plink, XLink::CreateDistinct( key_node ) );
        master_scr_engine->SetReplaceKey( link, KEY_PRODUCER_7 );
    }
    
    // Note that the keylink could have been set via coupling - but still not
    // likely to do anything sensible, so explicitly check
    return DuplicateSubtree(key_node);   
}
