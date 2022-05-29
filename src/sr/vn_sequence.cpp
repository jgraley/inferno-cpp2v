#include "vn_sequence.hpp"
#include "vn_transformation.hpp"


using namespace SR;

void VNSequence::Configure( const vector< shared_ptr<VNTransformation> > &sequence )
{
    steps = sequence;
}                                  


void VNSequence::PatternTransformations( int step_index )
{
    steps[step_index]->PatternTransformations();
}


void VNSequence::PlanningStageOne( int step_index )
{
    steps[step_index]->PlanningStageOne();
}


void VNSequence::PlanningStageTwo( int step_index )
{
    steps[step_index]->PlanningStageTwo();
}


void VNSequence::PlanningStageThree( int step_index )
{
    steps[step_index]->PlanningStageThree();
}


void VNSequence::SetMaxReps( int step_index, int n, bool e ) 
{ 
    VNTransformation::SetMaxReps(n, e);
}


void VNSequence::SetStopAfter( int step_index, vector<int> ssa, int d )
{
    steps[step_index]->SetStopAfter(ssa, d);
}  


void VNSequence::operator()( int step_index,
                             TreePtr<Node> context, 
                             TreePtr<Node> *proot )
{
    (*steps[step_index])(context, proot);
}                                   
                 
/*
Graphable::Block VNTransformation::GetGraphBlockInfo() const
{
    ASSERT( this )("Called on NULL pointer, I expect");
    ASSERT( top_level_engine )("VNTransformation needs to be configured before use");
    return top_level_engine->GetGraphBlockInfo();
}  


string VNSequence::GetGraphId() const
{
    ASSERT( this )("Called on NULL pointer, I expect");
    ASSERT( top_level_engine )("VNTransformation needs to be configured before use");
    return top_level_engine->GetGraphId();
}  
*/

void VNSequence::GenerateGraphRegions( int step_index, Graph &graph ) const
{
    steps[step_index]->GenerateGraphRegions(graph);
}  
