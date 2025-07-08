#include "builder_agent.hpp"
#include "scr_engine.hpp"
#include "sym/lazy_eval.hpp"
#include "sym/lazy_eval.hpp"
#include "sym/boolean_operators.hpp"
#include "helpers/simple_duplicate.hpp"

using namespace SR;


SYM::Lazy<SYM::BooleanExpression> BuilderAgent::SymbolicNormalLinkedQueryPRed() const
{
    // Match anything (subject to pre-restriction, MMAX and couplings)
    return SYM::MakeLazy<SYM::BooleanConstant>(true);
}                                      


bool BuilderAgent::ReplaceKeyerQuery( PatternLink me_plink, 
                                      set<PatternLink> keyer_plinks )
{
    ASSERT( me_plink.GetChildAgent() == this );
    ASSERT( !my_scr_engine->IsKeyed(me_plink) ); // should only be reached once for each plink
    bool should_key = !my_scr_engine->IsKeyed(this);
    
    if( should_key )
    {
        ASSERT( !keyer_plink );
        keyer_plink = me_plink;
    }
    
    return should_key; 
}
 
 
Agent::ReplacePatchPtr BuilderAgent::GenReplaceLayoutImpl( const ReplaceKit &kit, 
                                                           PatternLink me_plink, 
                                                           XLink key_xlink )
{
    INDENT("%");

    TreePtr<Node> new_node;
    if( me_plink == keyer_plink )
    {
        // Call the soft pattern impl 
        built_node = new_node = BuildNewSubtree();
        
        // Don't duplicate since this is first one     
        // Note: from #807 we don't now set a key with the SCR engine 
        // since tree update does so for all agents that submit patches.
        // This has allowed the removal of a CreateDistinct() call.
    }
    else if( key_xlink ) // residual but keyed during search or externally
    {        
        // Duplicate to keep free zones distinct since not first one
        new_node = SimpleDuplicate::DuplicateSubtree(key_xlink.GetChildTreePtr());
    }
    else // residual but seen earlier during this get-layout pass 
    {
        // Duplicate to keep free zones distinct since not first one
        new_node = SimpleDuplicate::DuplicateSubtree(built_node);
	}

    // Make free zone without duplicating since this is first one
    return make_shared<FreePatch>( FreeZone::CreateSubtree(new_node) );
}
