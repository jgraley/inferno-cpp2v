#include "builder_agent.hpp"
#include "scr_engine.hpp"
#include "sym/lazy_eval.hpp"
#include "sym/lazy_eval.hpp"
#include "sym/boolean_operators.hpp"
#include "helpers/simple_duplicate.hpp"

using namespace SR;


SYM::Lazy<SYM::BooleanExpression> BuilderAgent::SymbolicNormalLinkedQueryPRed(PatternLink keyer_plink) const
{
    // Match anything (subject to pre-restriction, MMAX and couplings)
    return SYM::MakeLazy<SYM::BooleanConstant>(true);
}                                      

 
Agent::ReplacePatchPtr BuilderAgent::GenReplaceLayoutImpl( const ReplaceKit &kit, 
                                                           PatternLink me_plink, 
                                                           XLink key_xlink,
                                                  const SCREngine *acting_engine )
{
    INDENT("%");

    TreePtr<Node> new_node;
    if( me_plink == keyer_plink ) // keyer
	{
        // Call the soft pattern impl 
        built_node = new_node = BuildNewSubtree(acting_engine); // acts like Clone()
        
        // Don't duplicate since this is first one     
        // Note: from #807 we don't now set a key with the SCR engine 
        // since tree update does so for all agents that submit patches.
    }
    else // residual
    {
		TreePtr<Node> key;
		if( key_xlink ) // residual but keyed during search or externally	
			key = key_xlink.GetChildTreePtr(); 
		else // residual but seen earlier during this get-layout pass 
			key = built_node;
		
		// Duplicate to keep free zones distinct since not first one
		new_node = SimpleDuplicate::DuplicateSubtree(key);
	}   

    // Make free zone without duplicating since this is first one
    return make_shared<FreePatch>( FreeZone::CreateSubtree(new_node) );
}
