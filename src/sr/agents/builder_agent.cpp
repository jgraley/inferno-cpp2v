#include "builder_agent.hpp"
#include "scr_engine.hpp"

using namespace SR;

bool BuilderAgent::ReplaceKeyerQuery( PatternLink me_plink, 
                                      set<PatternLink> keyer_plinks )
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

    if( me_plink == keyer_plink )
    {
        ASSERT( !key_node ); // we're on keyer plink
        // Call the soft pattern impl 
        TreePtr<Node> new_node = BuildNewSubtree();
          
        LocatedLink new_link( me_plink, XLink::CreateDistinct( new_node ) );
        master_scr_engine->SetReplaceKey( new_link );
        
        return DuplicateSubtree(new_node);   
    }
    else
    {
        ASSERT( key_node ); // we're on residual plink
        return AgentCommon::BuildReplaceImpl( me_plink, key_node );   
    }
}
