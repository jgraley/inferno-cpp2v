#include "inferno.hpp"

#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "parse/parse.hpp"  
#include "render/render.hpp"
#include "render/graph.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "tree/validate.hpp"
#include "steps/split_instance_declarations.hpp"
#include "steps/generate_implicit_casts.hpp"
#include "steps/generate_stacks.hpp"
#include "steps/test_steps.hpp"
#include "steps/lower_control_flow.hpp"
#include "steps/clean_up.hpp"
#include "steps/state_out.hpp"
#include "steps/fall_out.hpp"
#include "steps/systemc_detection.hpp"
#include "steps/to_sc_method.hpp"
#include "render/doc_graphs.hpp"
#include "unit_test.hpp"
#include "sr/search_replace.hpp"
#include "sr/csp/reference_solver.hpp"
#include "sr/vn_sequence.hpp"

#include <cstdlib>

using namespace Steps;

// Build a vector of transformations, in the order that we will run them
// (ordered by hand for now, until the auto sequencer is ready)
void BuildDefaultSequence( vector< shared_ptr<VNTransformation> > *sequence )
{
    ASSERT( sequence );
    
    // Test steps that change (fix) the tree - do these first so 
    // intermediates are used (requres EXPECTATION_RUN in test examples)
    {
        sequence->push_back( make_shared<FixCrazyNumber>() );
    }
    
    // SystemC detection, converts implicit SystemC to explicit. Always at the top
    // because we never want to process implicit SystemC.
    DetectAllSCTypes::Build(sequence);

    { // establish what is locally uncombable
        sequence->push_back( make_shared<DetectUncombableSwitch>() );
        sequence->push_back( make_shared<MakeAllForUncombable>() );
        sequence->push_back( make_shared<DetectCombableFor>() );
        sequence->push_back( make_shared<MakeAllBreakUncombable>() );
        sequence->push_back( make_shared<CleanupCompoundMulti>() );
        sequence->push_back( make_shared<DetectCombableBreak>() );
    }    
    { // Construct lowerings
        { // function call lowering (and function merging)
            sequence->push_back( make_shared<ExtractCallParams>() );
            sequence->push_back( make_shared<ExplicitiseReturn>() );
            sequence->push_back( make_shared<ReturnViaTemp>() );
            sequence->push_back( make_shared<AddLinkAddress>() );
            sequence->push_back( make_shared<ParamsViaTemps>() );
            sequence->push_back( make_shared<SplitInstanceDeclarations>() );
            sequence->push_back( make_shared<MoveInstanceDeclarations>() );
            sequence->push_back( make_shared<AutosToModule>() );
            sequence->push_back( make_shared<GenerateStacks>() );
            sequence->push_back( make_shared<MergeFunctions>() );
        }
   
        sequence->push_back( make_shared<BreakToGoto>() );
        sequence->push_back( make_shared<ForToWhile>() );
        sequence->push_back( make_shared<WhileToDo>() );
        sequence->push_back( make_shared<DoToIfGoto>() );
        
        sequence->push_back( make_shared<LogicalOrToIf>() );
        sequence->push_back( make_shared<LogicalAndToIf>() );
        sequence->push_back( make_shared<ConditionalOperatorToIf>() );
        sequence->push_back( make_shared<SwitchToIfGoto>() );
        sequence->push_back( make_shared<SplitInstanceDeclarations>() );
        sequence->push_back( make_shared<IfToIfGoto>() );
        // All remaining uncombables at the top level and in SUSP style
    }    
    { // Initial treatment of gotos and labels
        sequence->push_back( make_shared<NormaliseConditionalGotos>() );
        sequence->push_back( make_shared<CompactGotos>() );
    }        
    { // big round of cleaning up
        sequence->push_back( make_shared<ReduceVoidStatementExpression>() );
        //for( int i=0; i<2; i++ )
        // Ineffectual gotos, unused and duplicate labels result from compound tidy-up after construct lowering, but if not 
        // removed before AddGotoBeforeLabel, they will generate spurious states. We also remove dead code which can be exposed by
        // removal of unused labels - we must repeat because dead code removal can generate unused labels.
        sequence->push_back( make_shared<CleanupStatementExpression>() );
        for( int i=0; i<2; i++ )
        {
            sequence->push_back( make_shared<CleanupCompoundMulti>() );
            sequence->push_back( make_shared<CleanupCompoundSingle>() );
            sequence->push_back( make_shared<CleanupNop>() );
            sequence->push_back( make_shared<CleanupUnusedLabels>() );
            sequence->push_back( make_shared<CleanupDuplicateLabels>() );
            sequence->push_back( make_shared<CleanupIneffectualLabels>() );
            sequence->push_back( make_shared<CleanUpDeadCode>() );
        }
    }    
    { // transition to normalised lmap style
        sequence->push_back( make_shared<GotoAfterWait>() );
        sequence->push_back( make_shared<AddGotoBeforeLabel>() );
        //sequence->push_back( make_shared<EnsureBootstrap>() );
        sequence->push_back( make_shared<EnsureResetYield>() );
        sequence->push_back( make_shared<CleanupCompoundMulti>() );
        sequence->push_back( make_shared<AddStateLabelVar>() );
        sequence->push_back( make_shared<PlaceLabelsInArray>() );
#if 1
        sequence->push_back( make_shared<LabelTypeToEnum>() );
#else
        for( int i=0; i<2; i++ )
        {
            sequence->push_back( make_shared<LabelVarsToEnum>() );
            sequence->push_back( make_shared<SwapSubscriptConditionalOperator>() );
        }
#endif        
        sequence->push_back( make_shared<CleanupCompoundMulti>() );
    }    
    { // creating fallthrough machine
        for( int i=0; i<5; i++ )
        {
            sequence->push_back( make_shared<ApplyCombGotoPolicy>() );
            sequence->push_back( make_shared<ApplyYieldGotoPolicy>() );
        }
        sequence->push_back( make_shared<ApplyBottomPolicy>() );
        sequence->push_back( make_shared<ApplyLabelPolicy>() );
        sequence->push_back( make_shared<CleanupDuplicateLabels>() );
        sequence->push_back( make_shared<ApplyTopPolicy>() );
        sequence->push_back( make_shared<DetectSuperLoop>(false) );
        sequence->push_back( make_shared<DetectSuperLoop>(true) );
        sequence->push_back( make_shared<CleanupUnusedVariables>() );
    }
    { // optimsing fall though machine
        sequence->push_back( make_shared<LoopRotation>() );
    }
    { // transition to event driven style
        sequence->push_back( make_shared<InsertInferredYield>() );
        sequence->push_back( make_shared<AutosToModule>() );
        sequence->push_back( make_shared<TempsAndStaticsToModule>() );
        sequence->push_back( make_shared<DeclsToModule>() );
        sequence->push_back( make_shared<ThreadToMethod>() );
        sequence->push_back( make_shared<ExplicitiseReturns>() );
        sequence->push_back( make_shared<CleanupNestedIf>() );
    }
    { // final cleanups
        for( int i=0; i<2; i++ )
        {
            sequence->push_back( make_shared<CleanupUnusedLabels>() );
            sequence->push_back( make_shared<CleanupDuplicateLabels>() );
            sequence->push_back( make_shared<CleanupIneffectualLabels>() );
            sequence->push_back( make_shared<CleanUpDeadCode>() );
        }
    }
}


void BuildDocSequence( vector< shared_ptr<VNTransformation> > *sequence )
{
    ASSERT( sequence );
    sequence->push_back( make_shared<SlaveTest>() );
    sequence->push_back( shared_ptr<VNTransformation>( new SlaveTest2 ) );
    sequence->push_back( shared_ptr<VNTransformation>( new SlaveTest3 ) );
}


Inferno::Inferno( shared_ptr<VNSequence> vn_sequence_ ) :
    vn_sequence( vn_sequence_ ),
    plan(this)
{
}


Inferno::Plan::Plan(Inferno *algo_) :
    algo( algo_ )
{
    // ------------------------ Form steps plan -------------------------
    // Start a steps plan
    algo->vn_sequence->ForSteps( [&](int i)
    {
        steps.push_back( { i, ReadArgs::trace, ReadArgs::trace_hits, true, false } );        
    } );
    
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
          [this]()
          { 
              Parse{ ReadArgs::infile }( algo->program, &algo->program ); 
          } }
    );
    
    // Render output X
    Stage stage_render_X(
        { Progress::RENDERING, 
          false, ReadArgs::trace_hits, false, false,
          "Rendering output to code", 
          nullptr, [&]()
          { 
              Render{ ReadArgs::outfile }( &algo->program ); 
          } }
    );

    // Output a pattern graph
    Stage stage_pattern_graphs( 
        { Progress::RENDERING, 
          false, false, false, false,
          "Rendering pattern graphs",
          nullptr,  
          [this]()
          { 
              algo->GeneratePatternGraphs(); 
          } } 
    );
    
    // Output an intermediate/output graph
    Stage stage_X_graph(
        { Progress::RENDERING, 
          false, ReadArgs::trace_hits, false, false,
          "Rendering output to graph", 
          nullptr, 
          [this]()
          { 
              Graph( ReadArgs::outfile, ReadArgs::outfile )( algo->program ); 
          } }
    );
    
    // Dump the hit counts
    Stage stage_dump_hits(
        { Progress::RENDERING, 
          false, false, false, false,
          "Dumping hit counts", 
          nullptr, 
          [this]()
          { 
              HitCount::instance.Dump(); 
          } }
    );
            
    // Pattern transformations
    Stage stage_pattern_transformation( 
        { Progress::PATTERN_TRANS, 
          true, false, false, false,
          "Pattern transforming", 
          [this](const Step &sp)
          { 
              algo->vn_sequence->PatternTransformations(sp.step_index); 
          }, 
          nullptr } 
    ); 

    // Planning
    vector<Stage> stages_planning( {
        { Progress::PLANNING_ONE, 
          true, false, false, false,
          "Planning stage one", 
          [this](const Step &sp)
          { 
              algo->vn_sequence->PlanningStageOne(sp.step_index); 
          }, 
          nullptr },
        { Progress::PLANNING_TWO, 
          true, false, false, false,
          "Planning stage two", 
          [this](const Step &sp)
          { 
              algo->vn_sequence->PlanningStageTwo(sp.step_index); 
          }, 
          nullptr },
        { Progress::PLANNING_THREE, 
          true, false, false, false,
          "Planning stage three", 
          [this](const Step &sp)
          { 
              algo->vn_sequence->PlanningStageThree(sp.step_index); 
          }, 
          nullptr },    
        { Progress::PLANNING_FOUR, 
          ReadArgs::trace, false, false, false,
          "Planning stage four", 
          nullptr,
          [this]()
          { 
              algo->vn_sequence->PlanningStageFour(); 
          } },    
        { Progress::PLANNING_FIVE, 
          true, false, false, false,
          "Planning stage five", 
          [this](const Step &sp)
          { 
              algo->vn_sequence->PlanningStageFive(sp.step_index); 
          }, 
          nullptr }
    } );         
                
    // Analyse X tree
    Stage stage_analyse(
        { Progress::ANALYSING, 
          true, true, false, false,
          "Analysing", 
          nullptr,
          [this]()
          { 
              algo->vn_sequence->AnalysisStage(algo->program); 
          } }
    );
            
    // X transformation
    Stage stage_transform_X(
        { Progress::TRANSFORMING, 
          true, true, true, true,
          "Transforming", 
          [this](const Step &sp)
          { 
              algo->RunTransformationStep(sp); 
          }, 
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

    for( Stage &stage : stages_planning )
    {
        stages.push_back( stage );
        // If a pattern trace graph was requested, generate it immediately after last planning stage.
        if( &stage == &(stages_planning.back()) && generate_pattern_graphs && ReadArgs::graph_trace )
            stages.push_back( stage_pattern_graphs );     
        if( ShouldIQuitAfter(stage) )
            return;
    }

    if( ReadArgs::documentation_graphs || generate_pattern_graphs )
        return;

    if( ReadArgs::infile=="" )
    {
        fprintf(stderr, "No input file provided so performing planning only. -h for help.\n");     
        goto FINAL_TRACE;
    }

    stages.push_back( stage_parse_X );   
    if( ShouldIQuitAfter(stage_parse_X) ) 
        goto FINAL_RENDER;         
    // Now input has been parsed, we always want to render even if quitting early.  
    
    stages.push_back( stage_analyse );   
    if( ShouldIQuitAfter(stage_analyse) ) 
        goto FINAL_RENDER; 
        
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


void Inferno::RunStage( Stage stage )
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
        for( const Step &sp : plan.steps )
        {
            Progress(stage.progress_stage, sp.step_index).SetAsCurrent();
            Tracer::Enable( stage.allow_trace && sp.allow_trace ); 
            HitCount::Enable( stage.allow_hits && sp.allow_hits ); 
            if( stage.allow_reps && sp.allow_reps )
                VNSequence::SetMaxReps( sp.step_index, ReadArgs::repetitions, ReadArgs::rep_error );
            else
                VNSequence::SetMaxReps( sp.step_index, 100, true );
            if( stage.allow_stop && sp.allow_stop )
                vn_sequence->SetStopAfter(sp.step_index, ReadArgs::quitafter_counts, 0);
            stage.step_function(sp);
        }        
        break;
    }
}

    
void Inferno::GeneratePatternGraphs()
{
    if( ReadArgs::pattern_graph_name.back()=='/' )
    {
        string dir = ReadArgs::pattern_graph_name;
        for( const Step &sp : plan.steps )
        {
            Progress(Progress::RENDERING, sp.step_index).SetAsCurrent();
            string ss;
            if( !ReadArgs::documentation_graphs )
                ss = SSPrintf("%03d-", sp.step_index);
            string filepath = dir + ss + vn_sequence->GetStepName(sp.step_index) + ".dot";                                                       
            Graph g( filepath, vn_sequence->GetStepName(sp.step_index) );
            GenerateGraphRegions( sp, g );
        }
    }
    else
    {
        Step my_sp;
        bool found = false;
        if( ReadArgs::pattern_graph_name.empty() )
        {
            ASSERT( ReadArgs::pattern_graph_index >= 0 )("Negative step number is silly\n");
            ASSERT( ReadArgs::pattern_graph_index < plan.steps.size() )("There are only %d steps at present\n", plan.steps.size() );
            my_sp = plan.steps[ReadArgs::pattern_graph_index];
            found = true;
        }
        else
        {
            for( const Step &sp : plan.steps )
            {                    
                if( ReadArgs::pattern_graph_name.empty() ?
                    sp.step_index == ReadArgs::pattern_graph_index :
                    vn_sequence->GetStepName(sp.step_index) == ReadArgs::pattern_graph_name )
                {
                    my_sp = sp;
                    found = true;
                    break;
                }
            }
            if( !found ) // not found?
            {
                fprintf(stderr, "Cannot find specified steps. Steps are:\n" );  
                for( const Step &sp : plan.steps )
                {
                    string ss = SSPrintf("%03d-", sp.step_index);
                    string msg = ss+vn_sequence->GetStepName(sp.step_index);
                    fprintf( stderr, "%szn", msg.c_str() );
                }
                ASSERT(false);
            }
        }
        Progress(Progress::RENDERING, my_sp.step_index).SetAsCurrent();
        Graph g( ReadArgs::outfile, vn_sequence->GetStepName(my_sp.step_index) );
        GenerateGraphRegions( my_sp, g );
    }       
}


void Inferno::GenerateGraphRegions( const Step &sp, Graph &graph )
{
	vn_sequence->DoGraph( sp.step_index, graph );
	if( ReadArgs::graph_trace )    
        vn_sequence->GenerateGraphRegions(sp.step_index, graph);
}
   
   
void Inferno::RunTransformationStep(const Step &sp)
{
    if( !ReadArgs::trace_quiet )
        fprintf(stderr, "%s at T%03d-%s\n", ReadArgs::infile.c_str(), sp.step_index, vn_sequence->GetStepName(sp.step_index).c_str() ); 
    VNSequence *vp = vn_sequence.get(); 
    program = vn_sequence->TransformStep( sp.step_index, program );
    if( ReadArgs::output_all )
    {
        Render r( ReadArgs::outfile+SSPrintf("_%03d.cpp", sp.step_index) );
        r( &program );     
        Graph g( ReadArgs::outfile+SSPrintf("_%03d.dot", sp.step_index), 
                 ReadArgs::outfile+SSPrintf(" after T%03d-%s", sp.step_index, vn_sequence->GetStepName(sp.step_index).c_str()) );
        g( program );    
    }           
}


void Inferno::Run()
{    
    for( Stage stage : plan.stages )    
        RunStage(stage);    
}


bool Inferno::ShouldIQuitAfter(Stage stage)
{
    return ReadArgs::quitafter && 
           ReadArgs::quitafter_progress.GetStage()==stage.progress_stage;
}


int main( int argc, char *argv[] )
{
    // Check the command line arguments 
    ReadArgs( argc, argv );

    HitCount::instance.Check();
    Tracer::Enable( ReadArgs::trace );
    HitCount::Enable( ReadArgs::trace_hits );

    // Do self-tests (unit tests) if requested
    if( ReadArgs::unit_tests )
    {
        SelfTest();
        return EXIT_SUCCESS;
    }
    
    // Build a sequence of steps 
    Inferno::Stage stage_build_steps( 
        { Progress::BUILDING_STEPS }
    );
    Progress(Progress::BUILDING_STEPS).SetAsCurrent();    
    vector< shared_ptr<SR::VNTransformation> > sequence;
    if( !ReadArgs::trace_quiet )
		fprintf(stderr, "Building patterns\n"); 
    if( ReadArgs::documentation_graphs )
        BuildDocSequence( &sequence );
    else
        BuildDefaultSequence( &sequence );    
        
    // Maybe we want to stop after buolding the steps
    if( Inferno::ShouldIQuitAfter(stage_build_steps) )
        return EXIT_SUCCESS;    

    // No, so create VNSequence and Inferno instances and run it:
    // VNSequence contains the algrithms.
    // Inferno is just a harness that supports various execution 
    // scenarios based on command line args.
    auto vn_sequence = make_shared<SR::VNSequence>( sequence );
    Inferno inferno( vn_sequence );
    inferno.Run();
    //CSP::ReferenceSolver::DumpGSV();
    return EXIT_SUCCESS;
}

// TODO Consider multi-terminus Stuff and multi-root (StarStuff)

