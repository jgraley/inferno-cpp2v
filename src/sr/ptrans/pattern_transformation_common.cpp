#include "pattern_transformation_common.hpp"
#include "vn_step.hpp"
#include "agents/slave_agent.hpp"

using namespace SR;

PatternTransformationCommon::PatternKnowledge::PatternKnowledge( VNStep &vnt )
{
    vn_transformation = &vnt;
    top_level_engine = vnt.GetTopLevelEngine();
    
    all_plinks.clear();
    search_compare_root_pattern = top_level_engine->GetSearchComparePattern();
    if( search_compare_root_pattern )
    {
        search_compare_root_agent = Agent::AsAgent(search_compare_root_pattern);
        search_compare_root_plink = PatternLink::CreateDistinct( search_compare_root_pattern );
        WalkPattern( all_plinks, search_compare_root_plink );
    }    
    replace_root_pattern = top_level_engine->GetReplacePattern();
    if( replace_root_pattern )
    {
        replace_root_agent = Agent::AsAgent(replace_root_pattern);
        replace_root_plink = PatternLink::CreateDistinct( replace_root_pattern );
        WalkPattern( all_plinks, replace_root_plink );
    }
    
    all_agents.clear();
    plinks_to_agents.clear();
    for( PatternLink plink : all_plinks )
    {
        all_agents.insert( plink.GetChildAgent() ); 
        plinks_to_agents[plink.GetChildAgent()].insert(plink);
    }
    
    slave_agents.clear();
    for( Agent *agent : all_agents )        
        if( auto sa = dynamic_cast<SlaveAgent *>(agent) )
            slave_agents.insert( sa );
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
    const PatternKnowledge pk( vnt );
    
    DoPatternTransformation( pk );    
}


