#include "vn_sequence.hpp"

#include "vn_step.hpp"
#include "render/graph.hpp"
#include "db/x_tree_database.hpp"
#include "db/tree_update.hpp"

using namespace SR;


// Make trace easier to follow
//#define X_TREE_DB_EACH_STEP

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
    
    // Give that set to x_tree_db planning
    x_tree_db = make_shared<XTreeDatabase>(expressions);
}


void VNSequence::PlanningStageFive( int step_index )
{
    steps[step_index]->PlanningStageFive(x_tree_db);
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
    current_root_xlink = XLink::CreateDistinct(root);    
    ASSERT( x_tree_db )("Planning stage four should have created x_tree_db object");
#ifndef X_TREE_DB_EACH_STEP
	x_tree_db->FullBuild( current_root_xlink );
#endif        
}


TreePtr<Node> VNSequence::TransformStep( int step_index, TreePtr<Node> root )
{
    ASSERT( current_root_xlink.GetChildX()==root );
    dirty_grass.clear();	
	    
#ifdef X_TREE_DB_EACH_STEP
	x_tree_db->Build( current_root_xlink );
#endif        
    steps[step_index]->Transform( current_root_xlink );
    root = current_root_xlink.GetChildX();
    return root;   
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
    ASSERT( x_tree_db )("Planning stage four should have created x_tree_db object");
    return x_tree_db->UniquifyDomainExtension( xlink ); 
}


XLink VNSequence::FindDomainExtension( XLink xlink ) const
{
    ASSERT( x_tree_db )("Planning stage four should have created x_tree_db object");
    return x_tree_db->FindDomainExtension( xlink ); 
}


void VNSequence::ExtendDomainNewPattern( PatternLink base_plink )
{
    ASSERT( x_tree_db )("Planning stage four should have created x_tree_db object");  
    x_tree_db->ExtendDomainNewPattern( base_plink );
}


void VNSequence::ExecuteUpdateCommand( shared_ptr<UpdateCommand> cmd )
{
	UpdateCommand::ExecKit kit { current_root_xlink, x_tree_db.get() };
	cmd->Execute( kit );
}


void VNSequence::XTreeDbBuildMonolithic()
{
	x_tree_db->BuildMonolithic();
    x_tree_db->ExtendDomainNewX();
}


bool VNSequence::IsDirtyGrass( TreePtr<Node> node )
{
	return dirty_grass.count(node) > 0;
}


void VNSequence::AddDirtyGrass( TreePtr<Node> node )
{
	dirty_grass.insert(node);
}

