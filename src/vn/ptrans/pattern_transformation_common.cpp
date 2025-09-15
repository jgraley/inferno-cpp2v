#include "pattern_transformation_common.hpp"
#include "vn_step.hpp"
#include "agents/embedded_scr_agent.hpp"

using namespace VN;

PatternTransformationCommon::PatternKnowledge::PatternKnowledge( VNStep &vnt, const PatternTransformationCommon *trans )
{
    vn_transformation = &vnt;
    root_engine = vnt.GetTopLevelEngine();
    
    set<PatternLink> all_plinks; // PLink memory safety: local so freed before any changes to the pattern

    search_compare_root_pattern = root_engine->GetSearchComparePattern();
    replace_root_pattern = root_engine->GetEmbeddedReplacePattern();

    if( search_compare_root_pattern )
    {
        search_compare_root_agent = Agent::AsAgent(search_compare_root_pattern);
        search_compare_root_plink = PatternLink( &search_compare_root_pattern );
        WalkPattern( all_plinks, search_compare_root_plink );
    }    
    if( replace_root_pattern )
    {
        replace_root_agent = Agent::AsAgent(replace_root_pattern);
        replace_root_plink = PatternLink( &replace_root_pattern );
        WalkPattern( all_plinks, replace_root_plink );
    }
    
    all_pattern_nodes.clear();
    agents_to_incoming_plinks.clear();
	for( PatternLink plink : all_plinks )
	{
		all_pattern_nodes.insert( plink.GetPattern() );
		
		if( trans->RequireAgentsToIncomingPlinksMap() )
			agents_to_incoming_plinks[plink.GetChildAgent()].insert(plink);
	}
		
    embedded_scr_nodes.clear();
    for( TreePtr<Node> node : all_pattern_nodes )        
        if( dynamic_cast<EmbeddedSCRAgent *>(Agent::AsAgent(node)) )
            embedded_scr_nodes.insert( node );
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


