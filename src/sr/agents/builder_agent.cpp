#include "builder_agent.hpp"
#include "coupling.hpp"
#include "scr_engine.hpp"

using namespace SR;

TreePtr<Node> BuilderAgent::BuildReplaceImpl( PatternLink me_plink, 
                                              TreePtr<Node> under_node ) 
{
    INDENT("%");
    if( !under_node )
    {
        // Call the soft pattern impl 
        under_node = BuildNewSubtree();
        
        LocatedLink link( me_plink, XLink::CreateDistinct( under_node ) );
        master_scr_engine->SetReplaceKey( link, KEY_PRODUCER_7 );
    }
    
    // Note that the keylink could have been set via coupling - but still not
    // likely to do anything sensible, so explicitly check
    return DuplicateSubtree(under_node);   
}
