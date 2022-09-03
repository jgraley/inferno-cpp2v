#include "vn_step.hpp"
#include "search_replace.hpp"
#include "scr_engine.hpp"
#include "ptrans/combine_patterns.hpp"
#include "ptrans/search_to_compare.hpp"
#include "ptrans/split_disjunctions.hpp"
#include "vn_sequence.hpp"

using namespace SR;

void VNStep::Configure( StepType type,
                                  TreePtr<Node> scp,
                                  TreePtr<Node> rp )
{
    switch( type )
    {
        case SEARCH_REPLACE:
            top_level_engine = make_shared<SearchReplace>();
            break;
            
        case COMPARE_REPLACE:
            top_level_engine = make_shared<CompareReplace>();
            break;
            
        default:
            ASSERTFAIL("Silly");
    }       
    
    top_level_engine->Configure( scp, rp );
}                                  


PatternTransformationVector VNStep::GetAllPatternTrans()
{
    PatternTransformationVector ptv;
    // Transformations to make life easier for the user
    ptv.push_back( make_shared<CombinePatterns>() );
    ptv.push_back( make_shared<SearchToCompare>() );
    
    // Transformations to make life easier for the solver
    ptv.push_back( make_shared<SplitDisjunctions>() );
    return ptv;
}


void VNStep::PatternTransformations()
{
    ASSERT( this )("Called on NULL pointer, I expect");
    ASSERT( top_level_engine )("VNStep needs to be configured before use");
    PatternTransformationVector ptv = GetAllPatternTrans();
    
    ptv(*this);
}


void VNStep::PlanningStageOne( VNSequence *vn_sequence_ )
{
    ASSERT( this )("Called on NULL pointer, I expect");
    ASSERT( vn_sequence_ );
    ASSERT( top_level_engine )("VNStep needs to be configured before use");
    vn_sequence = vn_sequence_;
    top_level_engine->PlanningStageOne(vn_sequence);
}


void VNStep::PlanningStageTwo()
{
    ASSERT( this )("Called on NULL pointer, I expect");
    ASSERT( top_level_engine )("VNStep needs to be configured before use");
    top_level_engine->PlanningStageTwo();
}


void VNStep::PlanningStageThree()
{
    ASSERT( this )("Called on NULL pointer, I expect");
    ASSERT( top_level_engine )("VNStep needs to be configured before use");
    top_level_engine->PlanningStageThree();
}


void VNStep::PlanningStageFive( shared_ptr<const TheKnowledge> knowledge )
{
    ASSERT( this )("Called on NULL pointer, I expect");
    ASSERT( top_level_engine )("VNStep needs to be configured before use");
    top_level_engine->PlanningStageFive(knowledge);
}


void VNStep::SetMaxReps( int n, bool e ) 
{ 
    SCREngine::SetMaxReps(n, e); 
}


void VNStep::SetStopAfter( vector<int> ssa, int d )
{
    ASSERT( this )("Called on NULL pointer, I expect");
    ASSERT( top_level_engine )("VNStep needs to be configured before use");
    top_level_engine->SetStopAfter( ssa, d );
}  


void VNStep::Transform( XLink root_xlink )
{
    ASSERT( this )("Called on NULL pointer, I expect");
    ASSERT( top_level_engine )("VNStep needs to be configured before use");
    top_level_engine->Transform( root_xlink );
}                                   


Graphable::Block VNStep::GetGraphBlockInfo() const
{
    ASSERT( this )("Called on NULL pointer, I expect");
    ASSERT( top_level_engine )("VNStep needs to be configured before use");
    return top_level_engine->GetGraphBlockInfo();
}  


string VNStep::GetGraphId() const
{
    ASSERT( this )("Called on NULL pointer, I expect");
    ASSERT( top_level_engine )("VNStep needs to be configured before use");
    return top_level_engine->GetGraphId();
}  


void VNStep::GenerateGraphRegions( Graph &graph ) const
{
    ASSERT( this )("Called on NULL pointer, I expect");
    ASSERT( top_level_engine )("VNStep needs to be configured before use");
    return top_level_engine->GenerateGraphRegions( graph );
}  


shared_ptr<CompareReplace> VNStep::GetTopLevelEngine() const
{
    ASSERT( this )("Called on NULL pointer, I expect");
    ASSERT( top_level_engine )("VNStep needs to be configured before use");
    return top_level_engine;
}


void VNStep::SetTopLevelEngine( shared_ptr<CompareReplace> tle )
{
    ASSERT( this )("Called on NULL pointer, I expect");
    ASSERT( tle )("Bad tle ptr");
    top_level_engine = tle;
}

