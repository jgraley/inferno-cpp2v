#include "vn_sequence.hpp"

#include "vn_step.hpp"
#include "render/graph.hpp"
#include "db/x_tree_database.hpp"
#include "up/zone_commands.hpp"
#include "up/tree_update.hpp"

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
	// TODO factor all this somewhere else
    // Determine the full set of expressions across all the steps
    set< shared_ptr<SYM::BooleanExpression> > clauses;
    for( shared_ptr<VNStep> vnt : steps )
    {
        const SCREngine *root_scr_engine = vnt->GetTopLevelEngine()->GetRootEngine();
        if( !root_scr_engine )
            continue; // apparently wasn't planned, probably due to -q being specified. See #641
        set< shared_ptr<SYM::BooleanExpression> > step_exprs = root_scr_engine->GetExpressions();
        clauses = UnionOfSolo( clauses, step_exprs );
    }
    
    set<const SYM::Expression *> sub_exprs;
    for( shared_ptr<SYM::BooleanExpression> clause : clauses )
    {
		clause->ForDepthFirstWalk([&](const SYM::Expression *sub_expr)
		{
			sub_exprs.insert(sub_expr);
		} );
   	}
   	
   	lacing = make_shared<Lacing>();
    lacing->Build( sub_exprs );   
    
    domain_extenders = DomainExtension::DetermineExtenders(sub_exprs);
}


void VNSequence::PlanningStageFive( int step_index )
{
    steps[step_index]->PlanningStageFive(lacing);  
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
    XLink root_xlink = XLink::CreateDistinct(root);    
    
    x_tree_db = make_shared<XTreeDatabase>(root_xlink, lacing, domain_extenders);
    ASSERT( x_tree_db )("Analysis stage should have created x_tree_db object");    
    
#ifndef X_TREE_DB_EACH_STEP
	x_tree_db->InitialBuild();
#endif        	
}


TreePtr<Node> VNSequence::TransformStep( int step_index, TreePtr<Node> root )
{
    ASSERT( root == x_tree_db->GetMainRootNode() )("Unexpected root - re-run AnalysisStage() to change root");
    
    x_tree_db->ClearDirtyGrass();	
	    
#ifdef X_TREE_DB_EACH_STEP
	x_tree_db->InitialBuild();
#endif        

    steps[step_index]->SetXTreeDb( x_tree_db );
    steps[step_index]->Transform();
    
    return x_tree_db->GetMainRootNode();   
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


void VNSequence::UpdateUsingCommand( shared_ptr<Command> cmd )
{
    ASSERT( x_tree_db )("Analysis stage should have created x_tree_db object");    
    
	TreeUpdater( x_tree_db.get() ).TransformToIncrementalAndExecute( cmd );
    
    x_tree_db->PostUpdateActions();
}


void VNSequence::XTreeDbDump() const
{
    ASSERT( x_tree_db )("Analysis stage should have created x_tree_db object");    
    x_tree_db->Dump();
}


bool VNSequence::IsDirtyGrass( TreePtr<Node> node ) const
{
	return x_tree_db->IsDirtyGrass(node);
}


void VNSequence::AddDirtyGrass( TreePtr<Node> node ) const
{
	x_tree_db->AddDirtyGrass(node);
}

