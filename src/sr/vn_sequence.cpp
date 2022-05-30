#include "vn_sequence.hpp"
#include "vn_transformation.hpp"
#include "render/graph.hpp"
#include "the_knowledge.hpp"

using namespace SR;

VNSequence::VNSequence( const vector< shared_ptr<VNTransformation> > &sequence ) :
    steps( sequence ),
    knowledge( make_shared<TheKnowledge>() )
{
}                                  


void VNSequence::PatternTransformations( int step_index )
{
    steps[step_index]->PatternTransformations();
}


void VNSequence::PlanningStageOne( int step_index )
{
    steps[step_index]->PlanningStageOne(this);
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
                             TreePtr<Node> *proot )
{
    (*steps[step_index])(proot);
}                                   
                 
                 
void VNSequence::ForSteps( function<void(int)> body )
{
    for( int i=0; i<steps.size(); i++ )
        body( i );
}
                 

void VNSequence::DoGraph( int step_index, Graph &graph ) const
{
    graph( steps[step_index].get() );
}


void VNSequence::GenerateGraphRegions( int step_index, Graph &graph ) const
{
    steps[step_index]->GenerateGraphRegions(graph);
}  


string VNSequence::GetStepName( int step_index ) const
{
    return steps[step_index]->GetName();
}


const TheKnowledge *VNSequence::GetTheKnowledge()
{
    return knowledge.get();
}


XLink VNSequence::UniquifyDomainExtension( XLink xlink ) const
{
    return knowledge->UniquifyDomainExtension( xlink ); 
}


XLink VNSequence::FindDomainExtension( XLink xlink ) const
{
    return knowledge->FindDomainExtension( xlink ); 
}


void VNSequence::UpdateTheKnowledge( PatternLink root_plink, XLink root_xlink )
{
    knowledge->Update( root_plink, root_xlink );
}

