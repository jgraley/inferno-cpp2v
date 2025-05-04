#include "vn_sequence.hpp"

#include "vn_step.hpp"
#include "render/graph.hpp"
#include "db/x_tree_database.hpp"
#include "up/tree_update.hpp"

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
        clause->ForTreeDepthFirstWalk([&](const SYM::Expression *sub_expr)
        {
            sub_exprs.insert(sub_expr);
        } );
    }
       
    // Use thes eto establish the lacing
    lacing = make_shared<Lacing>();
    lacing->Build( sub_exprs );   
    
    // ...and to set up the domain extension channels (actioned in analysis stage)
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
    x_tree_db = make_shared<XTreeDatabase>(lacing, domain_extenders);
    x_tree_db->InitialBuild(root);
}


TreePtr<Node> VNSequence::TransformStep( int step_index )
{           
    ASSERT( x_tree_db )("Analysis stage should have created x_tree_db object");         

    steps[step_index]->SetXTreeDb( x_tree_db );
    steps[step_index]->Transform();
    
    return x_tree_db->GetMainRootNode();   
}
           
                 
void VNSequence::ForSteps( function<void(int)> body )
{
    for( vector< shared_ptr<VNStep> >::size_type i=0; i<steps.size(); i++ )
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


void VNSequence::UpdateUsingLayout( XLink origin_xlink, shared_ptr<Patch> source_layout )
{
    ASSERT( x_tree_db )("Analysis stage should have created x_tree_db object");    
    
    TreeUpdater( x_tree_db.get() ).TransformToIncrementalAndExecute( origin_xlink, source_layout );   
}


void VNSequence::XTreeDbDump() const
{
    ASSERT( x_tree_db )("Analysis stage should have created x_tree_db object");    
    x_tree_db->Dump();
}

