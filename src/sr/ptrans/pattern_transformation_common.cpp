#include "pattern_transformation_common.hpp"
#include "vn_transformation.hpp"
#include "agents/slave_agent.hpp"

using namespace SR;

PatternTransformationCommon::PatternKnowledge::PatternKnowledge( VNTransformation &vnt )
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
    
    slave_plinks.clear();
    for( PatternLink plink : all_plinks )
        if( auto sa = dynamic_cast<SlaveAgent *>(plink.GetChildAgent()) )
            slave_plinks.insert( plink );
    
    for( PatternLink plink : all_plinks )
        plinks_to_agents[plink.GetChildAgent()].insert(plink);
}


void PatternTransformationCommon::PatternKnowledge::WalkPattern( set<PatternLink> &all_plinks, 
                                                                 PatternLink plink ) const
{
    all_plinks.insert( plink );    
    list<PatternLink> child_plinks = plink.GetChildAgent()->GetChildren(); 
    for( PatternLink plink : child_plinks )
        WalkPattern( all_plinks, plink );    
}

   
void PatternTransformationCommon::operator()( VNTransformation &vnt )
{
    const PatternKnowledge pk( vnt );
    
    DoPatternTransformation( pk );    
}


