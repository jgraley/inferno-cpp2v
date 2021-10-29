#include "builder_agent.hpp"
#include "coupling.hpp"
#include "scr_engine.hpp"

using namespace SR;

TreePtr<Node> BuilderAgent::BuildReplaceImpl( PatternLink me_plink, 
                                              TreePtr<Node> key_node ) 
{
    INDENT("%");

    // Fails because of nodes keyed by master SCREngine - not seen in plan.
    //ASSERT( master_scr_engine->IsKeyedByAndRuleEngine(this)==!!key_node )
    //        (*this)(" got planned ")(master_scr_engine->IsKeyedByAndRuleEngine(this))
    //        (" but key is ")(key_node)("\n");

    // If keyed, we don't act, so revert to base class algo
    if( key_node )
        return AgentCommon::BuildReplaceImpl( me_plink, key_node );   

    // Call the soft pattern impl 
    TreePtr<Node> new_node = BuildNewSubtree();
      
    LocatedLink new_link( me_plink, XLink::CreateDistinct( new_node ) );
    master_scr_engine->SetReplaceKey( new_link, KEY_PRODUCER_7 );
    
    return DuplicateSubtree(new_node);   
}
