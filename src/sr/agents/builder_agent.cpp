#include "builder_agent.hpp"
#include "coupling.hpp"
#include "scr_engine.hpp"

using namespace SR;

bool BuilderAgent::PlanReplaceKeying( PatternLink me_plink, 
                                      unordered_set<PatternLink> keyer_plinks )
{
    ASSERT( me_plink.GetChildAgent() == this );
    ASSERT( !master_scr_engine->IsKeyed(me_plink) ); // should only be reached once for each plink
    bool should_key = !master_scr_engine->IsKeyed(this);
    
    if( should_key )
    {
        ASSERT( !keyer_plink );
        keyer_plink = me_plink;
    }
    
    return should_key; 
}
 
 
TreePtr<Node> BuilderAgent::BuildReplaceImpl( PatternLink me_plink, 
                                              TreePtr<Node> key_node ) 
{
    INDENT("%");

    // Fails because of nodes keyed by master SCREngine - not seen in plan.
    //ASSERT( master_scr_engine->IsKeyedByAndRuleEngine(this)==!!key_node )
    //        (*this)(" got planned ")(master_scr_engine->IsKeyedByAndRuleEngine(this))
    //        (" but key is ")(key_node)("\n");

    // If keyed, we don't act, so revert to base class algo
    //ASSERT( !key_node == (keyer_plink==me_plink) );

    if( key_node )
        return AgentCommon::BuildReplaceImpl( me_plink, key_node );   

    // Call the soft pattern impl 
    TreePtr<Node> new_node = BuildNewSubtree();
      
    LocatedLink new_link( me_plink, XLink::CreateDistinct( new_node ) );
    master_scr_engine->SetReplaceKey( new_link, KEY_PRODUCER_7 );
    
    return DuplicateSubtree(new_node);   
}
