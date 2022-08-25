#include "vn_sequence.hpp"
#include "vn_step.hpp"
#include "render/graph.hpp"
#include "the_knowledge.hpp"

using namespace SR;

VNSequence::VNSequence( const vector< shared_ptr<VNStep> > &sequence ) :
    steps( sequence )
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


void VNSequence::PlanningStageFour()
{
    // Determine the full set of expressions across all the steps
    set< shared_ptr<SYM::BooleanExpression> > expressions;
    for( shared_ptr<VNStep> vnt : steps )
    {
        const SCREngine *root_scr_engine = vnt->GetTopLevelEngine()->GetRootEngine();
        if( !root_scr_engine )
            continue; // apparently wasn't planned, probably due to -q being specified.
        set< shared_ptr<SYM::BooleanExpression> > step_exprs = root_scr_engine->GetExpressions();
        expressions = UnionOfSolo( expressions, step_exprs );
    }
    
    // Give that set to knowledge planning
    knowledge = make_shared<TheKnowledge>(expressions);
}


void VNSequence::PlanningStageFive( int step_index )
{
    steps[step_index]->PlanningStageFive(knowledge);
}


void VNSequence::SetMaxReps( int step_index, int n, bool e ) 
{ 
    VNStep::SetMaxReps(n, e);
}


void VNSequence::SetStopAfter( int step_index, vector<int> ssa, int d )
{
    steps[step_index]->SetStopAfter(ssa, d);
}  


void VNSequence::AnalysisStage( TreePtr<Node> root )
{
#ifdef NEW_KNOWLEDGE_UPDATE
    ASSERT( knowledge )("Planning stage four should have created knowledge object");
    initial_root_xlink = XLink::CreateDistinct(root);    
    knowledge->Build( root_xlink );
#endif    
}


TreePtr<Node> VNSequence::TransformStep( int step_index, TreePtr<Node> root )
{
#ifdef XLINK_CONTINUITY
    if( step_index == 0 )
    {
        ASSERT( initial_root_xlink );
        ASSERT( initial_root_xlink.GetChildX()==root );
        steps[step_index]->Transform( initial_root_xlink );
        initial_root_xlink = XLink();
        return initial_root_xlink.GetChildX(); // could have changed 
    }
    else
#endif
    {        
        ASSERT( !initial_root_xlink );
        steps[step_index]->Transform( &root );
        return root;
    }    
}
           
                 
void VNSequence::ForSteps( function<void(int)> body )
{
    for( int i=0; i<steps.size(); i++ )
        body( i );
}
                 

void VNSequence::DoGraph( int step_index, Graph &graph ) const
{
    graph.GenerateGraph( steps[step_index].get() );
}


void VNSequence::GenerateGraphRegions( int step_index, Graph &graph ) const
{
    steps[step_index]->GenerateGraphRegions(graph);
}  


string VNSequence::GetStepName( int step_index ) const
{
    return steps[step_index]->GetName();
}


XLink VNSequence::UniquifyDomainExtension( XLink xlink ) const
{
    ASSERT( knowledge )("Planning stage four should have created knowledge object");
    return knowledge->UniquifyDomainExtension( xlink ); 
}


XLink VNSequence::FindDomainExtension( XLink xlink ) const
{
    ASSERT( knowledge )("Planning stage four should have created knowledge object");
    return knowledge->FindDomainExtension( xlink ); 
}


void VNSequence::BuildTheKnowledge( XLink root_xlink )
{
    ASSERT( knowledge )("Planning stage four should have created knowledge object");
    knowledge->Build( root_xlink );
}


void VNSequence::ExtendDomain( PatternLink root_plink )
{
    ASSERT( knowledge )("Planning stage four should have created knowledge object");
    knowledge->ExtendDomain( root_plink );
}
