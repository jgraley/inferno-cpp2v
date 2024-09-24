#include "builder_agent.hpp"
#include "scr_engine.hpp"
#include "sym/lazy_eval.hpp"
#include "sym/lazy_eval.hpp"
#include "sym/boolean_operators.hpp"
#include "up/commands.hpp"
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
 
 
Agent::FreeZoneExprPtr BuilderAgent::GenFreeZoneExprImpl( const ReplaceKit &kit, 
                                                     PatternLink me_plink, 
                                                     XLink key_xlink )
{
    INDENT("%");

	TreePtr<Node> new_node;
    if( me_plink == keyer_plink )
    {
        ASSERT( !key_xlink ); // we're on keyer plink
        // Call the soft pattern impl 
        new_node = BuildNewSubtree();
          
        // Key it
        XLink new_xlink = XLink::CreateDistinct( new_node );
        my_scr_engine->SetReplaceKey( LocatedLink( me_plink, new_xlink ) );   
        
        // Don't duplicate since this is first one     
    }
    else
    {
        ASSERT( key_xlink ); // we're on residual plink
        
        // Duplicate to keep free zones distinct since not first one
        new_node = SimpleDuplicate::DuplicateSubtree(key_xlink.GetChildX());
    }

    // Make free zone without duplicating since this is first one
    return make_shared<PopulateFreeZoneOperator>( FreeZone::CreateSubtree( new_node ) );
}
