#include "vn_transformation.hpp"
#include "search_replace.hpp"
#include "scr_engine.hpp"
#include "ptrans/combine_patterns.hpp"
#include "ptrans/search_to_compare.hpp"

using namespace SR;

void VNTransformation::Configure( TransformationType type,
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


PatternTransformationVector VNTransformation::GetAllPatternTrans()
{
    PatternTransformationVector ptv;
    ptv.push_back( make_shared<CombinePatterns>() );
    ptv.push_back( make_shared<SearchToCompare>() );
    return ptv;
}


void VNTransformation::PatternTransformations()
{
    ASSERT( this )("Called on NULL pointer, I expect");
    ASSERT( top_level_engine )("VNTransformation needs to be configured before use");
    PatternTransformationVector ptv = GetAllPatternTrans();
    
    ptv(*this);
}


void VNTransformation::PlanningStageOne()
{
    ASSERT( this )("Called on NULL pointer, I expect");
    ASSERT( top_level_engine )("VNTransformation needs to be configured before use");
    top_level_engine->PlanningStageOne();
}


void VNTransformation::PlanningStageTwo()
{
    ASSERT( this )("Called on NULL pointer, I expect");
    ASSERT( top_level_engine )("VNTransformation needs to be configured before use");
    top_level_engine->PlanningStageTwo();
}


void VNTransformation::PlanningStageThree()
{
    ASSERT( this )("Called on NULL pointer, I expect");
    ASSERT( top_level_engine )("VNTransformation needs to be configured before use");
    top_level_engine->PlanningStageThree();
}


void VNTransformation::SetMaxReps( int n, bool e ) 
{ 
    SCREngine::SetMaxReps(n, e); 
}


void VNTransformation::SetStopAfter( vector<int> ssa, int d )
{
    ASSERT( this )("Called on NULL pointer, I expect");
    ASSERT( top_level_engine )("VNTransformation needs to be configured before use");
    top_level_engine->SetStopAfter( ssa, d );
}  


void VNTransformation::operator()( TreePtr<Node> context, 
                                   TreePtr<Node> *proot )
{
    ASSERT( this )("Called on NULL pointer, I expect");
    ASSERT( top_level_engine )("VNTransformation needs to be configured before use");
    top_level_engine->operator()( context, proot );
}                                   
                 

Graphable::Block VNTransformation::GetGraphBlockInfo( const LinkNamingFunction &lnf,
                                                      const NonTrivialPreRestrictionFunction &ntprf ) const
{
    ASSERT( this )("Called on NULL pointer, I expect");
    ASSERT( top_level_engine )("VNTransformation needs to be configured before use");
    return top_level_engine->GetGraphBlockInfo( lnf, ntprf );
}  


string VNTransformation::GetGraphId() const
{
    ASSERT( this )("Called on NULL pointer, I expect");
    ASSERT( top_level_engine )("VNTransformation needs to be configured before use");
    return top_level_engine->GetGraphId();
}  


void VNTransformation::GenerateGraphRegions( Graph &graph ) const
{
    ASSERT( this )("Called on NULL pointer, I expect");
    ASSERT( top_level_engine )("VNTransformation needs to be configured before use");
    return top_level_engine->GenerateGraphRegions( graph );
}  


shared_ptr<CompareReplace> VNTransformation::GetTopLevelEngine() const
{
    ASSERT( this )("Called on NULL pointer, I expect");
    ASSERT( top_level_engine )("VNTransformation needs to be configured before use");
    return top_level_engine;
}


void VNTransformation::SetTopLevelEngine( shared_ptr<CompareReplace> tle )
{
    ASSERT( this )("Called on NULL pointer, I expect");
    ASSERT( tle )("Bad tle ptr");
    top_level_engine = tle;
}
