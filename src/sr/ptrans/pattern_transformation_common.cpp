#include "pattern_transformation_common.hpp"
#include "vn_transformation.hpp"
#include "agents/slave_agent.hpp"

using namespace SR;


void PatternTransformationCommon::operator()( VNTransformation &vnt )
{
    PatternKnowledge pk;
    pk.vn_transformation = &vnt;
    pk.top_level_engine = vnt.GetTopLevelEngine();
    
    pk.all_plinks.clear();
    pk.search_compare_root_pattern = pk.top_level_engine->GetSearchComparePattern();
    if( pk.search_compare_root_pattern )
    {
        pk.search_compare_root_agent = Agent::AsAgent(pk.search_compare_root_pattern);
        pk.search_compare_root_plink = PatternLink::CreateDistinct( pk.search_compare_root_pattern );
        WalkPattern( pk.all_plinks, pk.search_compare_root_plink );
    }    
    pk.replace_root_pattern = pk.top_level_engine->GetReplacePattern();
    if( pk.replace_root_pattern )
    {
        pk.replace_root_agent = Agent::AsAgent(pk.replace_root_pattern);
        pk.replace_root_plink = PatternLink::CreateDistinct( pk.replace_root_pattern );
        WalkPattern( pk.all_plinks, pk.replace_root_plink );
    }
    
    pk.slave_plinks.clear();
    for( PatternLink plink : pk.all_plinks )
        if( auto sa = dynamic_cast<SlaveAgent *>(plink.GetChildAgent()) )
            pk.slave_plinks.insert( plink );
    
    DoPatternTransformation( pk );    
    
    vnt.SetTopLevelEngine(pk.top_level_engine); // in case trans changed it
}


void PatternTransformationCommon::WalkPattern( set<PatternLink> &all_plinks, 
                                               PatternLink plink ) const
{
    all_plinks.insert( plink );    
    list<PatternLink> child_plinks = plink.GetChildAgent()->GetChildren(); 
    for( PatternLink plink : child_plinks )
        WalkPattern( all_plinks, plink );    
}

   