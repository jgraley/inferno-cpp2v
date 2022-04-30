#include "inferno.hpp"

#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "parse/parse.hpp"  
#include "render/render.hpp"
#include "render/graph.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "sr/search_replace.hpp"
#include "tree/validate.hpp"
#include "steps/split_instance_declarations.hpp"
#include "steps/generate_implicit_casts.hpp"
#include "steps/generate_stacks.hpp"
#include "steps/slave_test.hpp"
#include "steps/lower_control_flow.hpp"
#include "steps/clean_up.hpp"
#include "steps/state_out.hpp"
#include "steps/fall_out.hpp"
#include "steps/systemc_detection.hpp"
#include "steps/to_sc_method.hpp"
#include "render/doc_graphs.hpp"
#include "unit_test.hpp"
#include "sr/csp/reference_solver.hpp"

#include <cstdlib>

using namespace Steps;

// Build a vector of transformations, in the order that we will run them
// (ordered by hand for now, until the auto sequencer is ready)
void BuildDefaultSequence( vector< shared_ptr<Transformation> > *sequence )
{
    ASSERT( sequence );
    // SystemC detection, converts implicit SystemC to explicit. Always at the top
    // because we never want to process implicit SystemC.
    DetectAllSCTypes::Build(sequence);

    { // establish what is locally uncombable
        sequence->push_back( shared_ptr<Transformation>( new DetectUncombableSwitch ) );
        sequence->push_back( shared_ptr<Transformation>( new MakeAllForUncombable ) );
        sequence->push_back( shared_ptr<Transformation>( new DetectCombableFor ) );
        sequence->push_back( shared_ptr<Transformation>( new MakeAllBreakUncombable ) );
        sequence->push_back( shared_ptr<Transformation>( new CleanupCompoundMulti ) );    // for DetectCombableBreak
        sequence->push_back( shared_ptr<Transformation>( new DetectCombableBreak ) );
    }    
    { // Construct lowerings
        { // function call lowering (and function merging)
            sequence->push_back( shared_ptr<Transformation>( new ExtractCallParams ) ); 
            sequence->push_back( shared_ptr<Transformation>( new ExplicitiseReturn ) );
            sequence->push_back( shared_ptr<Transformation>( new ReturnViaTemp ) );
            sequence->push_back( shared_ptr<Transformation>( new AddLinkAddress ) );
            sequence->push_back( shared_ptr<Transformation>( new ParamsViaTemps ) );
            sequence->push_back( shared_ptr<Transformation>( new SplitInstanceDeclarations ) ); 
            sequence->push_back( shared_ptr<Transformation>( new MoveInstanceDeclarations ) ); 
            sequence->push_back( shared_ptr<Transformation>( new AutosToModule ) );
            sequence->push_back( shared_ptr<Transformation>( new GenerateStacks ) );
            sequence->push_back( shared_ptr<Transformation>( new MergeFunctions ) );
        }
   
        sequence->push_back( shared_ptr<Transformation>( new BreakToGoto ) ); 
        sequence->push_back( shared_ptr<Transformation>( new ForToWhile ) ); 
        sequence->push_back( shared_ptr<Transformation>( new WhileToDo ) ); 
        sequence->push_back( shared_ptr<Transformation>( new DoToIfGoto ) ); 
        
        sequence->push_back( shared_ptr<Transformation>( new LogicalOrToIf ) ); 
        sequence->push_back( shared_ptr<Transformation>( new LogicalAndToIf ) ); 
        sequence->push_back( shared_ptr<Transformation>( new ConditionalOperatorToIf ) ); 
        sequence->push_back( shared_ptr<Transformation>( new SwitchToIfGoto ) );
        sequence->push_back( shared_ptr<Transformation>( new SplitInstanceDeclarations ) );  
        sequence->push_back( shared_ptr<Transformation>( new IfToIfGoto ) ); 
        // All remaining uncombables at the top level and in SUSP style
    }    
    { // Initial treatment of gotos and labels
        sequence->push_back( shared_ptr<Transformation>( new NormaliseConditionalGotos ) );
        sequence->push_back( shared_ptr<Transformation>( new CompactGotos ) ); // maybe put these after the label cleanups
    }        
    { // big round of cleaning up
        sequence->push_back( shared_ptr<Transformation>( new ReduceVoidStatementExpression ) ); 
        //for( int i=0; i<2; i++ )
        // Ineffectual gotos, unused and duplicate labels result from compound tidy-up after construct lowering, but if not 
        // removed before AddGotoBeforeLabel, they will generate spurious states. We also remove dead code which can be exposed by
        // removal of unused labels - we must repeat because dead code removal can generate unused labels.
        sequence->push_back( shared_ptr<Transformation>( new CleanupStatementExpression ) ); // TODO only safe in SSP, so don't call this a cleanup!
        for( int i=0; i<2; i++ )
        {
            sequence->push_back( shared_ptr<Transformation>( new CleanupCompoundMulti ) );
            sequence->push_back( shared_ptr<Transformation>( new CleanupCompoundSingle ) ); 
            sequence->push_back( shared_ptr<Transformation>( new CleanupNop ) ); 
            sequence->push_back( shared_ptr<Transformation>( new CleanupUnusedLabels ) ); 
            sequence->push_back( shared_ptr<Transformation>( new CleanupDuplicateLabels ) );
            sequence->push_back( shared_ptr<Transformation>( new CleanupIneffectualLabels ) ); 
            sequence->push_back( shared_ptr<Transformation>( new CleanUpDeadCode ) ); 
        }
    }    
    { // transition to normalised lmap style
        sequence->push_back( shared_ptr<Transformation>( new GotoAfterWait ) );     
        sequence->push_back( shared_ptr<Transformation>( new AddGotoBeforeLabel ) );         
        //sequence->push_back( shared_ptr<Transformation>( new EnsureBootstrap ) );     
        sequence->push_back( shared_ptr<Transformation>( new EnsureResetYield ) );
        sequence->push_back( shared_ptr<Transformation>( new CleanupCompoundMulti ) );
        sequence->push_back( shared_ptr<Transformation>( new AddStateLabelVar ) ); 
        sequence->push_back( shared_ptr<Transformation>( new PlaceLabelsInArray ) );  
#if 1
        sequence->push_back( shared_ptr<Transformation>( new LabelTypeToEnum ) ); 
#else
        for( int i=0; i<2; i++ )
        {
            sequence->push_back( shared_ptr<Transformation>( new LabelVarsToEnum ) ); 
            sequence->push_back( shared_ptr<Transformation>( new SwapSubscriptConditionalOperator ) );        
        }
#endif        
        sequence->push_back( shared_ptr<Transformation>( new CleanupCompoundMulti ) );
    }    
    { // creating fallthrough machine
        for( int i=0; i<5; i++ )
        {
            sequence->push_back( shared_ptr<Transformation>( new ApplyCombGotoPolicy ) );
            sequence->push_back( shared_ptr<Transformation>( new ApplyYieldGotoPolicy ) );
        }
        sequence->push_back( shared_ptr<Transformation>( new ApplyBottomPolicy ) );
        sequence->push_back( shared_ptr<Transformation>( new ApplyLabelPolicy ) );
        sequence->push_back( shared_ptr<Transformation>( new CleanupDuplicateLabels ) );
        sequence->push_back( shared_ptr<Transformation>( new ApplyTopPolicy ) );
        sequence->push_back( shared_ptr<Transformation>( new DetectSuperLoop(false) ) );
        sequence->push_back( shared_ptr<Transformation>( new DetectSuperLoop(true) ) );
        sequence->push_back( shared_ptr<Transformation>( new CleanupUnusedVariables ) );
    }
    { // optimsing fall though machine
        sequence->push_back( shared_ptr<Transformation>( new LoopRotation ) );
    }
    { // transition to event driven style
        sequence->push_back( shared_ptr<Transformation>( new InsertInferredYield ) );
        sequence->push_back( shared_ptr<Transformation>( new AutosToModule ) );
        sequence->push_back( shared_ptr<Transformation>( new TempsAndStaticsToModule ) ); // TODO why?
        sequence->push_back( shared_ptr<Transformation>( new DeclsToModule ) );
        sequence->push_back( shared_ptr<Transformation>( new ThreadToMethod ) );
        sequence->push_back( shared_ptr<Transformation>( new ExplicitiseReturns ) );   // TODO move this into Verilog phase?
        sequence->push_back( shared_ptr<Transformation>( new CleanupNestedIf ) );      // and this
    }
    { // final cleanups
        for( int i=0; i<2; i++ )
        {
            sequence->push_back( shared_ptr<Transformation>( new CleanupUnusedLabels ) ); 
            sequence->push_back( shared_ptr<Transformation>( new CleanupDuplicateLabels ) );
            sequence->push_back( shared_ptr<Transformation>( new CleanupIneffectualLabels ) ); 
            sequence->push_back( shared_ptr<Transformation>( new CleanUpDeadCode ) ); 
        }
    }
}


void BuildDocSequence( vector< shared_ptr<Transformation> > *sequence )
{
    ASSERT( sequence );
    sequence->push_back( shared_ptr<Transformation>( new SlaveTest ) );
    sequence->push_back( shared_ptr<Transformation>( new SlaveTest2 ) );
    sequence->push_back( shared_ptr<Transformation>( new SlaveTest3 ) );
}


void GenerateGraphRegions( Graph &graph, shared_ptr<Transformation> t )
{
	graph( t.get() );
	if( ReadArgs::graph_trace )
    {
		if( auto vnt = dynamic_pointer_cast<VNTransformation>(t) )
			vnt->GenerateGraphRegions(graph);
        else
            ASSERTFAIL("Don't know how to do a trace-mode graph of this");
    }
}
   
   
Inferno::Inferno() :
    plan(this)
{
}


Inferno::Plan::Plan(Inferno *algo_) :
    algo( algo_ )
{
    // ------------------------ Form steps plan -------------------------
    Stage stage_build_steps( 
        { Progress::BUILDING_STEPS }
    );

    Progress(Progress::BUILDING_STEPS).SetAsCurrent();    

    vector< shared_ptr<Transformation> > sequence;
    // Build the sequence of steps
    if( !ReadArgs::trace_quiet )
		fprintf(stderr, "Building patterns\n"); 
    if( ReadArgs::documentation_graphs )
        BuildDocSequence( &sequence );
    else
        BuildDefaultSequence( &sequence );
    if( ShouldIQuitAfter(stage_build_steps) )
        return;    

    // Start a steps plan
    for( int i=0; i<sequence.size(); i++ )
        steps.push_back( { sequence[i], i, ReadArgs::trace, ReadArgs::trace_hits, true, false } );        
    
    // If we're to run only one step, restrict all stepped stages
    if( ReadArgs::runonlyenable )
        steps = { steps[ReadArgs::runonlystep] };

    // If we're to quit after a particular step, restrict all stepped stages
    if( ReadArgs::quitafter )
    {
        if( ReadArgs::quitafter_progress.GetStep() != Progress::NO_STEP )
        {
            steps.resize(ReadArgs::quitafter_progress.GetStep() + 1);
            steps.back().allow_stop = true;
        }
        for( int i=0; i<steps.size()-1; i++ )
            steps[i].allow_trace = steps[i].allow_hits = steps[i].allow_reps = steps[i].allow_stop = false;
        for( int i=0; i<steps.size(); i++ )
            TRACE("Step %03d ALLOWS: trace=", i)
                 (steps[i].allow_trace)(" hits=")
                 (steps[i].allow_hits)(" reps=")
                 (steps[i].allow_reps)(" stop=")
                 (steps[i].allow_stop)("\n");
    }

    // ------------------------ Create stages -------------------------
              
    // Parse input X
    Stage stage_parse_X(
        { Progress::PARSING, 
          ReadArgs::trace, false, false, false,
          SSPrintf("Parsing input %s", ReadArgs::infile.c_str()), 
          nullptr, 
          [this](){ Parse{ ReadArgs::infile }( algo->program, &algo->program ); } }
    );
    
    // Render output X
    Stage stage_render_X(
        { Progress::RENDERING, 
          false, ReadArgs::trace_hits, false, false,
          "Rendering output to code", 
          nullptr, [&](){ Render{ ReadArgs::outfile }( &algo->program ); } }
    );

    // Output a pattern graph
    Stage stage_pattern_graphs( 
        { Progress::RENDERING, 
          false, false, false, false,
          "Rendering pattern graphs",
          nullptr,  
          [this](){ algo->GeneratePatternGraphs(); } } 
    );
    
    // Output an intermediate/output graph
    Stage stage_X_graph(
        { Progress::RENDERING, 
          false, ReadArgs::trace_hits, false, false,
          "Rendering output to graph", 
          nullptr, 
          [this](){ Graph( ReadArgs::outfile, ReadArgs::outfile )( &algo->program ); } }
    );
    
    // Dump the hit counts
    Stage stage_dump_hits(
        { Progress::RENDERING, 
          false, false, false, false,
          "Dumping hit counts", 
          nullptr, 
          [this](){ HitCount::instance.Dump(); } }
    );
            
    // Pattern transformations
    Stage stage_pattern_transformation( 
        { Progress::PATTERN_TRANS, 
          true, false, false, false,
          "Pattern transforming", 
          [this](shared_ptr<VNTransformation> pvnt, const Plan::Step &sp){ pvnt->PatternTransformations(); }, 
          nullptr } 
    ); 

    // Planning
    vector<Stage> stages_planning( {
        { Progress::PLANNING_ONE, 
          true, false, false, false,
          "Planning stage one", 
          [this](shared_ptr<VNTransformation> pvnt, const Plan::Step &sp){ pvnt->PlanningStageOne(); }, 
          nullptr },
        { Progress::PLANNING_TWO, 
          true, false, false, false,
          "Planning stage two", 
          [this](shared_ptr<VNTransformation> pvnt, const Plan::Step &sp){ pvnt->PlanningStageTwo(); }, 
          nullptr },
        { Progress::PLANNING_THREE, 
          true, false, false, false,
          "Planning stage three", 
          [this](shared_ptr<VNTransformation> pvnt, const Plan::Step &sp){ pvnt->PlanningStageThree(); }, 
          nullptr }    
    } );         
                
    // X transformation
    Stage stage_transform_X(
        { Progress::TRANSFORMING, 
          true, true, true, true,
          "Transforming", 
          [this](shared_ptr<VNTransformation> pvnt, const Plan::Step &sp){ algo->RunTransformationStep(pvnt, sp); }, 
          nullptr }
    );
            
    // ------------------------ Form stages plan -------------------------
    stages.clear();
    bool generate_pattern_graphs = !ReadArgs::pattern_graph_name.empty() || 
                                   ReadArgs::pattern_graph_index != -1;
                                   
    if( generate_pattern_graphs && !ReadArgs::graph_trace )
        stages.push_back( stage_pattern_graphs );    
                
    stages.push_back( stage_pattern_transformation );         
    if( ShouldIQuitAfter(stage_pattern_transformation) )
        return;

    stages.push_back( stages_planning[0] );     
    if( ShouldIQuitAfter(stages_planning[0]) )
        return;

    stages.push_back( stages_planning[1] );     
    if( ShouldIQuitAfter(stages_planning[1]) )
        return;
    
    stages.push_back( stages_planning[2] );           
    // If a pattern trace graph was requested, generate it now. We need the
    // agents to have been configured (planning stage 2)
    if( generate_pattern_graphs && ReadArgs::graph_trace )
        stages.push_back( stage_pattern_graphs );        
    if( ShouldIQuitAfter(stages_planning[2]) || 
        ReadArgs::documentation_graphs || 
        generate_pattern_graphs)
        return;

    if( ReadArgs::infile=="" )
    {
        fprintf(stderr, "No input file provided so performing planning only. -h for help.\n");     
        goto FINAL_TRACE;
    }

    stages.push_back( stage_parse_X );   
    if( ShouldIQuitAfter(stage_parse_X) ) 
        goto FINAL_RENDER; // Now input has been parsed, we always want to render even if quitting early.    
    
    stages.push_back( stage_transform_X );        
    if( ShouldIQuitAfter(stage_transform_X) ) 
        goto FINAL_RENDER;
        
    FINAL_RENDER:
    if( ReadArgs::intermediate_graph && !ReadArgs::output_all )
        stages.push_back( stage_X_graph );
    else if( !ReadArgs::output_all )   
        stages.push_back( stage_render_X );          
        
    FINAL_TRACE:
    if( ReadArgs::trace_hits )
        stages.push_back( stage_dump_hits );
}


bool Inferno::Plan::ShouldIQuitAfter(Stage stage)
{
    return ReadArgs::quitafter && 
           ReadArgs::quitafter_progress.GetStage()==stage.progress_stage;
}


void Inferno::RunStage( Plan::Stage stage )
{
    if( !ReadArgs::trace_quiet )
		fprintf(stderr, "%s\n", stage.text.c_str());     
    
    switch( Progress(stage.progress_stage).GetSteppiness() )
    {
    case Progress::NON_STEPPY:
        Progress(stage.progress_stage).SetAsCurrent();
        Tracer::Enable( stage.allow_trace ); 
        HitCount::Enable( stage.allow_hits ); 
        stage.stage_function();
        break;
    
    case Progress::STEPPY:        
        for( const Plan::Step &sp : plan.steps )
        {
            Progress(stage.progress_stage, sp.step_index).SetAsCurrent();
            Tracer::Enable( stage.allow_trace && sp.allow_trace ); 
            HitCount::Enable( stage.allow_hits && sp.allow_hits ); 
            if( stage.allow_reps && sp.allow_reps )
                VNTransformation::SetMaxReps( ReadArgs::repetitions, ReadArgs::rep_error );
            else
                VNTransformation::SetMaxReps( 100, true );
            if( stage.allow_stop && sp.allow_stop )
                sp.tx->SetStopAfter(ReadArgs::quitafter_counts, 0);
            if( auto pvnt = dynamic_pointer_cast<VNTransformation>(sp.tx) )
                stage.step_function(pvnt, sp);
            else
                ASSERTFAIL("Unknown transformation");
        }        
        break;
    }
}

    
void Inferno::GeneratePatternGraphs()
{
    if( ReadArgs::pattern_graph_name.back()=='/' )
    {
        string dir = ReadArgs::pattern_graph_name;
        for( const Plan::Step &sp : plan.steps )
        {
            Progress(Progress::RENDERING, sp.step_index).SetAsCurrent();
            string ss;
            if( !ReadArgs::documentation_graphs )
                ss = SSPrintf("%03d-", sp.step_index);
            string filepath = dir + ss + sp.tx->GetName() + ".dot";                                                       
            Graph g( filepath, sp.tx->GetName() );
            GenerateGraphRegions( g, sp.tx );
        }
    }
    else
    {
        Plan::Step my_sp;
        if( ReadArgs::pattern_graph_name.empty() )
        {
            ASSERT( ReadArgs::pattern_graph_index >= 0 )("Negative step number is silly\n");
            ASSERT( ReadArgs::pattern_graph_index < plan.steps.size() )("There are only %d steps at present\n", plan.steps.size() );
            my_sp = plan.steps[ReadArgs::pattern_graph_index];
        }
        else
        {
            for( const Plan::Step &sp : plan.steps )
            {                    
                if( ReadArgs::pattern_graph_name.empty() ?
                    sp.step_index == ReadArgs::pattern_graph_index :
                    sp.tx->GetName() == ReadArgs::pattern_graph_name )
                {
                    my_sp = sp;
                    break;
                }
            }
            if( !my_sp.tx ) // not found?
            {
                fprintf(stderr, "Cannot find specified steps. Steps are:\n" );  
                for( const Plan::Step &sp : plan.steps )
                {
                    string ss = SSPrintf("%03d-", sp.step_index);
                    string msg = ss+sp.tx->GetName();
                    fprintf( stderr, "%szn", msg.c_str() );
                }
                ASSERT(false);
            }
        }
        Progress(Progress::RENDERING, my_sp.step_index).SetAsCurrent();
        Graph g( ReadArgs::outfile, my_sp.tx->GetName() );
        GenerateGraphRegions( g, my_sp.tx );
    }       
}


void Inferno::RunTransformationStep(shared_ptr<SR::VNTransformation> pvnt, const Plan::Step &sp)
{
    if( !ReadArgs::trace_quiet )
        fprintf(stderr, "%s at T%03d-%s\n", ReadArgs::infile.c_str(), sp.step_index, sp.tx->GetName().c_str() ); 
    (*pvnt)( &program );
    if( ReadArgs::output_all )
    {
        Render r( ReadArgs::outfile+SSPrintf("_%03d.cpp", sp.step_index) );
        r( &program );     
        Graph g( ReadArgs::outfile+SSPrintf("_%03d.dot", sp.step_index), 
                 ReadArgs::outfile+SSPrintf(" after T%03d-%s", sp.step_index, sp.tx->GetName().c_str()) );
        g( &program );    
    }           
}


void Inferno::Run()
{    
    for( Plan::Stage stage : plan.stages )    
        RunStage(stage);    
}


int main( int argc, char *argv[] )
{
    // Check the command line arguments 
    ReadArgs( argc, argv );

    HitCount::instance.Check();
    Tracer::Enable( ReadArgs::trace );
    HitCount::Enable( ReadArgs::trace_hits );

    // Do self-tests (unit tests) if requested
    if( ReadArgs::selftest )
    {
        SelfTest();
        return EXIT_SUCCESS;
    }
    
    Inferno inferno;
    inferno.Run();
    //CSP::ReferenceSolver::DumpGSV();
    return EXIT_SUCCESS;
}

// TODO Make Filter a functor. 
// TODO Consider merging Filter into Transformation.
// TODO Consider multi-terminus Stuff and multi-root (StarStuff)

