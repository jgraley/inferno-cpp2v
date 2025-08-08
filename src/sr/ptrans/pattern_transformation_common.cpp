#include "pattern_transformation_common.hpp"
#include "vn_step.hpp"
#include "agents/embedded_scr_agent.hpp"

using namespace SR;

PatternTransformationCommon::PatternKnowledge::PatternKnowledge( VNStep &vnt, const PatternTransformationCommon *trans )
{
    vn_transformation = &vnt;
    root_engine = vnt.GetTopLevelEngine();
    
    set<PatternLink> all_plinks; // PLink memory safety: local so freed before any changes to the pattern

    sp_tp_search_compare_root_pattern = make_shared<TreePtr<Node>>( root_engine->GetSearchComparePattern() );
    sp_tp_replace_root_pattern = make_shared<TreePtr<Node>>( root_engine->GetEmbeddedReplacePattern() );


    if( *sp_tp_search_compare_root_pattern )
    {
        search_compare_root_agent = Agent::AsAgent(*sp_tp_search_compare_root_pattern);
        search_compare_root_plink = PatternLink( sp_tp_search_compare_root_pattern );
        WalkPattern( all_plinks, search_compare_root_plink );
    }    
    if( *sp_tp_replace_root_pattern )
    {
        replace_root_agent = Agent::AsAgent(*sp_tp_replace_root_pattern);
        replace_root_plink = PatternLink( sp_tp_replace_root_pattern );
        WalkPattern( all_plinks, replace_root_plink );
    }
    
    all_agents.clear();
    agents_to_incoming_plinks.clear();
	for( PatternLink plink : all_plinks )
	{
		all_agents.insert( plink.GetChildAgent() ); 
		
		if( trans->RequireAgentsToIncomingPlinksMap() )
			agents_to_incoming_plinks[plink.GetChildAgent()].insert(plink);
	}
		
    embedded_scr_agents.clear();
    for( Agent *agent : all_agents )        
        if( auto sa = dynamic_cast<EmbeddedSCRAgent *>(agent) )
            embedded_scr_agents.insert( sa );
}


void PatternTransformationCommon::PatternKnowledge::WalkPattern( set<PatternLink> &all_plinks, 
                                                                 PatternLink plink ) const
{
    all_plinks.insert( plink );    
    list<PatternLink> child_plinks = plink.GetChildAgent()->GetChildren(); 
    for( PatternLink plink : child_plinks )
        WalkPattern( all_plinks, plink );    
}

   
void PatternTransformationCommon::operator()( VNStep &vnt )
{
    const PatternKnowledge pk( vnt, this );
    
    DoPatternTransformation( pk );    
}


