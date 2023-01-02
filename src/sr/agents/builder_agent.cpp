#include "builder_agent.hpp"
#include "scr_engine.hpp"
#include "sym/lazy_eval.hpp"
#include "sym/lazy_eval.hpp"
#include "sym/boolean_operators.hpp"
#include "db/tree_update.hpp"

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
 
 
Agent::CommandPtr BuilderAgent::GenerateCommandImpl( const ReplaceKit &kit, 
                                                     PatternLink me_plink, 
                                                     XLink key_xlink )
{
    INDENT("%");

    if( me_plink == keyer_plink )
    {
        ASSERT( !key_xlink ); // we're on keyer plink
        // Call the soft pattern impl 
        TreePtr<Node> new_node = BuildNewSubtree();
          
        LocatedLink new_link( me_plink, XLink::CreateDistinct( new_node ) );
        my_scr_engine->SetReplaceKey( new_link );
        
        auto new_zone = TreeZone::CreateSubtree( new_link );
		auto commands = make_unique<CommandSequence>();
		commands->Add( make_unique<DuplicateTreeZoneCommand>( new_zone ) );
		commands->Add( make_unique<PopulateFreeZoneCommand>() );
		return commands;
    }
    else
    {
        ASSERT( key_xlink ); // we're on residual plink
        auto new_zone = TreeZone::CreateSubtree( key_xlink );
		auto commands = make_unique<CommandSequence>();
		commands->Add( make_unique<DuplicateTreeZoneCommand>( new_zone ) );
		commands->Add( make_unique<PopulateFreeZoneCommand>() );
		return commands;
    }
}
