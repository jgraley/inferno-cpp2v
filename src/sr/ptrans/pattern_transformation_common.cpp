#include "pattern_transformation_common.hpp"
#include "vn_transformation.hpp"
#include "agents/slave_agent.hpp"

using namespace SR;


void PatternTransformationCommon::operator()( VNTransformation &vnt )
{
    Info info;
    info.vn_transformation = &vnt;
    info.top_level_engine = vnt.GetTopLevelEngine();
    
    info.children.clear();
    if( info.top_level_engine->GetSearchComparePattern() )
        WalkPattern( info.children, Agent::AsAgent(info.top_level_engine->GetSearchComparePattern()) );
    if( info.top_level_engine->GetReplacePattern() )
        WalkPattern( info.children, Agent::AsAgent(info.top_level_engine->GetReplacePattern()) );
    
    info.slaves.clear();
    for( Agent *a : info.children )
        if( auto sa = dynamic_cast<SlaveAgent *>(a) )
            info.slaves.insert( sa );
    
    DoPatternTransformation( info );    
    
    vnt.SetTopLevelEngine(info.top_level_engine); // in case trans changed it
}


void PatternTransformationCommon::WalkPattern( set<Agent *> &children, 
                                               Agent *agent ) const
{
    children.insert( agent );    
    list<PatternLink> plinks = agent->GetChildren(); 
    for( PatternLink plink : plinks )
        WalkPattern( children, plink.GetChildAgent() );    
}

   