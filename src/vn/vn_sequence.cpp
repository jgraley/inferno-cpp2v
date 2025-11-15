#include "vn_sequence.hpp"

#include "vn_step.hpp"
#include "graph/graph.hpp"
#include "db/x_tree_database.hpp"
#include "up/tree_update.hpp"
#include "up/up_common.hpp"

using namespace VN;

VNSequence::VNSequence( const vector< shared_ptr<VNStep> > &sequence ) :
    steps( sequence )
{
}                                  


VNSequence::~VNSequence()
{
	if( tree_updater ) // got past analysis
	{
		tree_updater->TeardownMainTree();
	}
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
       
    // Use these to establish the lacing
    lacing = make_shared<Lacing>();
    lacing->Build( sub_exprs );   
    
    // ...and to set up the domain extension channels (actioned in analysis stage)
    domain_extenders = DomainExtension::DetermineExtenders(sub_exprs);
}


void VNSequence::PlanningStageFive( int step_index )
{
    steps[step_index]->PlanningStageFive(lacing);  
}


void VNSequence::SetMaxReps( int n, bool e ) 
{ 
    VNStep::SetMaxReps(n, e);
}


void VNSequence::SetStopAfter( int step_index, vector<int> ssa, int d )
{
    steps[step_index]->SetStopAfter(ssa, d);
}  


void VNSequence::AnalysisStage( TreePtr<Node> main_tree_root )
{        
    x_tree_db = make_shared<XTreeDatabase>(lacing, domain_extenders);
    tree_updater = make_unique<TreeUpdater>(x_tree_db.get()); 
    
    // This will initialise all the DB assets including orderings
    // and domain extension, and so counts as an "analysis" of the
    // input tree.
    tree_updater->BuildMainTree(main_tree_root);
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
    string name = steps[step_index]->GetName();
    ASSERT( name != "VNStep" )("VNStep needs to bo overridden by built-in or soft step");
    return name;
}


void VNSequence::DoRender( int step_index, Render &render ) const
{
    steps[step_index]->DoRender(render);
}


TreeUpdater *VNSequence::GetTreeUpdater() const
{
	return tree_updater.get();
}


void VNSequence::XTreeDbDump() const
{
    ASSERT( x_tree_db )("Analysis stage should have created x_tree_db object");    
    x_tree_db->Dump();
}

