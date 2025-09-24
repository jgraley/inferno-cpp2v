#include "inferno.hpp"

#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "cplusplus/parse.hpp"  
#include "cplusplus/cpprender.hpp"  
#include "vn/lang/render.hpp"
#include "vn/graph/graph.hpp"
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
#include "steps/systemc_generation.hpp"
#include "steps/systemc_lowering.hpp"
#include "steps/to_sc_method.hpp"
#include "vn/graph/doc_graphs.hpp"
#include "unit_test.hpp"
#include "vn/search_replace.hpp"
#include "vn/csp/reference_solver.hpp"
#include "vn/vn_sequence.hpp"

#include <cstdlib>

//#define TEST_754
//#define REPRODUCE_833

using namespace Steps;

// Build a vector of transformations, in the order that we will run them
// (ordered by hand for now, until the auto sequencer is ready)
void BuildDefaultSequence( vector< shared_ptr<VNStep> > *sequence )
{
    ASSERT( sequence );
        
#ifdef TEST_754
    sequence->push_back( make_shared<DroppedTreeZone>() );
    return;
#endif

    // Test steps that change (fix) the tree - do these first so 
    // intermediates are used (requres EXPECTATION_RUN in test examples)
    {
        sequence->push_back( make_shared<FixCrazyNumber>() );
        sequence->push_back( make_shared<FixCrazyNumberEmb>() );
    }
    
    // SystemC detection, converts implicit SystemC to explicit. Always at the top
    // because we cannot render+compile implicit SystemC.
    DetectAllSystemC::Build(sequence);
    
    // SystemC generation tries to convert C and/or C++ into SystemC. This
    // is a simplification of what would happen in ealy phases of the original
    // Inferno design. Explicit SC nodes are generated.
    GenerateSC::Build(sequence);

    { 
		// ---------------------- Establish what is locally uncombable ----------------------
        sequence->push_back( make_shared<DetectUncombableSwitch>() );
        sequence->push_back( make_shared<MakeAllForUncombable>() );
        sequence->push_back( make_shared<DetectCombableFor>() );
        sequence->push_back( make_shared<MakeAllBreakUncombable>() );
        sequence->push_back( make_shared<CleanupCompoundMulti>() );
        sequence->push_back( make_shared<DetectCombableBreak>() );
    }    
    { 
		// ---------------------- Function merging ----------------------
		// Note: not the same as inlining: we are building stacks
		// for recursion, and turning calls and returns into gotos.
		// There is no duplication from multiple call sites, and
		// no limit on recursion (aside from stack size). We can do
		// this before lowering the structured programming constructs
		// because we make use of statement expressions. 
		sequence->push_back( make_shared<FunctionMergingDisallowed>() );
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
	
#ifdef REPRODUCE_833 // this cleanup is desirable to make function-merge output readable but fix #833 first
  	// ---------------------- big round of cleaning up ----------------------
	sequence->push_back( make_shared<CleanupVoidStatementExpression>() );
	sequence->push_back( make_shared<CleanupStatementExpression>() );
	// Ineffectual gotos, unused and duplicate labels result from compound tidy-up after construct lowering, but if not 
	// removed before AddGotoBeforeLabel, they will generate spurious states. We also remove dead code which can be exposed by
	// removal of unused labels - we must repeat because dead code removal can generate unused labels.
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
#endif

	{
		// ---------------------- Construct lowerings ----------------------	
		// Lower structured programming constructs and &&, ||, ?:
		// NOTE: After this sub-phase, it won't be possible to add usages of 
		// these constructs, which is why we leave this as late as possible.
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
        // All remaining uncombables at the top level and in SUSP style (Simple Uncombable Sequence Points)
    }    

	// ---------------------- big round of cleaning up ----------------------
	sequence->push_back( make_shared<CleanupVoidStatementExpression>() );
	sequence->push_back( make_shared<CleanupStatementExpression>() );
	// Ineffectual gotos, unused and duplicate labels result from compound tidy-up after construct lowering, but if not 
	// removed before AddGotoBeforeLabel, they will generate spurious states. We also remove dead code which can be exposed by
	// removal of unused labels - we must repeat because dead code removal can generate unused labels.
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
   
    { 
		// ---------------------- Install state enum and lmap ----------------------
        sequence->push_back( make_shared<GotoAfterWait>() );
        sequence->push_back( make_shared<AddGotoBeforeLabel>() );
		sequence->push_back( make_shared<NormaliseConditionalGotos>() );
		sequence->push_back( make_shared<CompactGotos>() );
        sequence->push_back( make_shared<EnsureResetYield>() );
        sequence->push_back( make_shared<CleanupCompoundMulti>() );
        sequence->push_back( make_shared<AddStateLabelVar>() );
        sequence->push_back( make_shared<PlaceLabelsInArray>() );
        sequence->push_back( make_shared<LabelTypeToEnum>() );     
    }    

    sequence->push_back( make_shared<CleanupCompoundMulti>() );

    { 
		// ---------------------- Create fallthrough machine ----------------------
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
    }

    sequence->push_back( make_shared<CleanupUnusedVariables>() );
    
    { 
		// ---------------------- Optimsing fall though machine ----------------------
        sequence->push_back( make_shared<LoopRotation>() );
    }
    
    { 
		// ---------------------- Transition to event driven style ----------------------
        sequence->push_back( make_shared<InsertInferredYield>() );
        sequence->push_back( make_shared<AutosToModule>() );
        sequence->push_back( make_shared<TempsAndStaticsToModule>() );
        sequence->push_back( make_shared<DeclsToModule>() );
        sequence->push_back( make_shared<ThreadToMethod>() );
        sequence->push_back( make_shared<ExplicitiseReturns>() );
        sequence->push_back( make_shared<CleanupNestedIf>() );
    }
    
	// ---------------------- Final cleanups ----------------------
    for( int i=0; i<2; i++ )
	{
		sequence->push_back( make_shared<CleanupUnusedLabels>() );
		sequence->push_back( make_shared<CleanupDuplicateLabels>() );
		sequence->push_back( make_shared<CleanupIneffectualLabels>() );
		sequence->push_back( make_shared<CleanUpDeadCode>() );
	}
	
	LowerAllSystemC::Build(sequence);
}


void BuildDocSequence( vector< shared_ptr<VNStep> > *sequence )
{
    ASSERT( sequence );
    sequence->push_back( make_shared<EmbeddedSCRTest>() );
    sequence->push_back( shared_ptr<VNStep>( new EmbeddedSCRTest2 ) );
    sequence->push_back( shared_ptr<VNStep>( new EmbeddedSCRTest3 ) );
}


Inferno::Inferno( shared_ptr<VNSequence> vn_sequence_ ) :
    vn_sequence( vn_sequence_ ),
    plan(this)
{
}


Inferno::~Inferno()
{
	//FTRACE("hi\n");
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
    if( ReadArgs::quitafter &&
        ReadArgs::quitafter_progress.GetStep() != Progress::NO_STEP )
    {
        steps.resize(ReadArgs::quitafter_progress.GetStep() + 1);
        steps.back().allow_stop = true;
        for( vector<Step>::size_type i=0; i<steps.size()-1; i++ )
            steps[i].allow_trace = steps[i].allow_hits = steps[i].allow_reps = steps[i].allow_stop = false;        
        for( vector<Step>::size_type i=0; i<steps.size(); i++ )
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
          true, false, false, false,
          SSPrintf("Parsing input %s", ReadArgs::infile.c_str()), 
          nullptr, 
          [this]()
          { 
              Parse p( ReadArgs::infile );
              algo->program = p.DoParse(); 
          } }
    );
    
    // Render output X
    Stage stage_render_X(
        { Progress::RENDERING, 
          false, true, false, false,
          "Rendering output to code", 
          nullptr, [&]()
          { 
              CppRender r( ReadArgs::outfile );
              r.WriteToFile( r.RenderToString( algo->program ) ); 
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
			  using namespace std::placeholders;
			  algo->PatternDispatcher( bind(&Inferno::DoPatternGraph, algo, _1, _2, _3, _4), 
			  					       ReadArgs::pattern_graph_index,
								       ReadArgs::pattern_graph_name,
								       !ReadArgs::documentation_graphs );
          } } 
    );
    
    // Output a pattern render
    Stage stage_pattern_renders( 
        { Progress::RENDERING, 
          false, false, false, false,
          "Rendering pattern to VN lang",
          nullptr,  
          [this]()
          { 
			  using namespace std::placeholders;
			  algo->PatternDispatcher( bind(&Inferno::DoPatternRender, algo, _1, _2, _3, _4), 
	                                   ReadArgs::pattern_render_index,
	                                   ReadArgs::pattern_render_name );
          } } 
    );
    
    // Output an intermediate/output graph
    Stage stage_X_graph(
        { Progress::RENDERING, 
          false, true, false, false,
          "Rendering output to graph", 
          nullptr, 
          [this]()
          { 
              Graph g( ReadArgs::outfile, ReadArgs::outfile );
              g.GenerateGraph( algo->program ); 
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
          true, false, false, false,
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
    bool generate_pattern_renders = !ReadArgs::pattern_render_name.empty() || 
                                    ReadArgs::pattern_render_index != -1;
                                   
    if( generate_pattern_graphs && !ReadArgs::graph_trace )
        stages.push_back( stage_pattern_graphs );    
                
    stages.push_back( stage_pattern_transformation );         
    if( ShouldIQuitAfter(stage_pattern_transformation.progress_stage) )
        return;

    for( Stage &stage : stages_planning )
    {
        // Actions on all planning stages
        stages.push_back( stage );
        
        // Actions on last planning stage       
        if( &stage == &(stages_planning.back()) )
        { 
			// Pattern graphs genned after pattern transformation in trace mode only
			if( generate_pattern_graphs && ReadArgs::graph_trace )
				stages.push_back( stage_pattern_graphs );
				
			// VN language always rendered after pattern transformation
			if( generate_pattern_renders )
				stages.push_back( stage_pattern_renders );
        }
         
        if( ShouldIQuitAfter(stage.progress_stage) )
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
    if( ShouldIQuitAfter(stage_parse_X.progress_stage) ) 
        goto FINAL_RENDER;         
    // Now input has been parsed, we always want to render even if quitting early.  
    
    stages.push_back( stage_analyse );   
    if( ShouldIQuitAfter(stage_analyse.progress_stage) ) 
        goto FINAL_RENDER; 
        
    stages.push_back( stage_transform_X );        
    if( ShouldIQuitAfter(stage_transform_X.progress_stage) ) 
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
        Tracer::Enable( stage.allow_trace && ReadArgs::trace ); 
        HitCount::Enable( stage.allow_hits && ReadArgs::trace_hits ); 
        stage.stage_function();
        break;
    
    case Progress::STEPPY:        
        for( const Step &sp : plan.steps )
        {
            Progress(stage.progress_stage, sp.step_index).SetAsCurrent();
            Tracer::Enable( stage.allow_trace && sp.allow_trace ); 
            HitCount::Enable( stage.allow_hits && sp.allow_hits ); 
            if( stage.allow_reps && sp.allow_reps )
                VNSequence::SetMaxReps( ReadArgs::repetitions, ReadArgs::rep_error );
            else
                VNSequence::SetMaxReps( 100, true );
            if( stage.allow_stop && sp.allow_stop )
                vn_sequence->SetStopAfter(sp.step_index, ReadArgs::quitafter_counts, 0);
            stage.step_function(sp);
        }        
        break;
    }
}

    
void Inferno::PatternDispatcher(PatternAction action, int pattern_index, string pattern_name, bool prepend_step_number)
{
    if( pattern_name.back()=='/' )
    {
        string dir = pattern_name;
        for( const Step &sp : plan.steps )
        {
            Progress(Progress::RENDERING, sp.step_index).SetAsCurrent();
            string ss;
            if( prepend_step_number )
                ss = SSPrintf("%03d-", sp.step_index);
            string name = ss + vn_sequence->GetStepName(sp.step_index);
            fprintf(stderr, "%s\n", name.c_str() );            
            action( sp, dir + name, true, vn_sequence->GetStepName(sp.step_index) );
        }
    }
    else
    {
        Step my_sp;
        bool found = false;
        if( pattern_name.empty() )
        {
            ASSERT( pattern_index >= 0 )("Negative step number is silly\n");
            ASSERT( pattern_index < (int)(plan.steps.size()) )("There are only %d steps at present\n", plan.steps.size() );
            my_sp = plan.steps[pattern_index];
            found = true;
        }
        else
        {
            for( const Step &sp : plan.steps )
            {                    
                if( pattern_name.empty() ?
                    sp.step_index == pattern_index :
                    vn_sequence->GetStepName(sp.step_index) == pattern_name )
                {
                    my_sp = sp;
                    found = true;
                    break;
                }
            }
            if( !found ) // not found?
            {
                fprintf(stderr, "Cannot find step:\n%s\nSteps are:\n", pattern_name.c_str() );  
                for( const Step &sp : plan.steps )
                {
                    string msg = vn_sequence->GetStepName(sp.step_index);
                    msg += SSPrintf(" (%03d)", sp.step_index);
                    fprintf( stderr, "%s\n", msg.c_str() );
                }
                ASSERT(false);
            }
        }
        Progress(Progress::RENDERING, my_sp.step_index).SetAsCurrent();        
		action( my_sp, ReadArgs::outfile, false, vn_sequence->GetStepName(my_sp.step_index) );
    }       
}


void Inferno::DoPatternGraph( const Step &sp, string outfile, bool add_file_extension, string title ) const
{
	if( add_file_extension )
		outfile += ".dot";
	Graph graph( outfile, title );
    vn_sequence->DoGraph( sp.step_index, graph );
    if( ReadArgs::graph_trace )    
        vn_sequence->GenerateGraphRegions(sp.step_index, graph);
}
   
 
void Inferno::DoPatternRender( const Step &sp, string outfile, bool add_file_extension, string title ) const
{
	(void)title;
	if( add_file_extension )
		outfile += ".vn";
    Render r( outfile );
    vn_sequence->DoRender( sp.step_index, r );
}
   
   
void Inferno::RunTransformationStep(const Step &sp)
{
    if( !ReadArgs::trace_quiet )
        fprintf(stderr, "%s at T%03d-%s\n", ReadArgs::infile.c_str(), sp.step_index, vn_sequence->GetStepName(sp.step_index).c_str() ); 
    program = vn_sequence->TransformStep( sp.step_index );
    if( ReadArgs::output_all )
    {
        Render r( ReadArgs::outfile+SSPrintf("_%03d.cpp", sp.step_index) );
        r.WriteToFile( r.RenderToString( program ) );     
        Graph g( ReadArgs::outfile+SSPrintf("_%03d.dot", sp.step_index), 
                 ReadArgs::outfile+SSPrintf(" after T%03d-%s", sp.step_index, vn_sequence->GetStepName(sp.step_index).c_str()) );
        g.GenerateGraph( program );    
    }           
}


void Inferno::Run()
{    
    for( Stage stage : plan.stages )    
        RunStage(stage);    
}


bool Inferno::ShouldIQuitAfter(Progress::Stage stage)
{
    return ReadArgs::quitafter && 
           ReadArgs::quitafter_progress.GetStage()==stage;
}


int main( int argc, char *argv[] )
{
    // Check the command line arguments 
    ReadArgs( argc, argv );

    HitCount::instance.Check();
    Tracer::Enable( ReadArgs::trace );
    HitCount::Enable( ReadArgs::trace_hits );

    // Do self-tests (unit tests) if requested
    if( ReadArgs::test_units )
    {
        SelfTest();
        return EXIT_SUCCESS;
    }
    
    // Build a sequence of steps 
    Progress(Progress::BUILDING_STEPS).SetAsCurrent();    
    vector< shared_ptr<VN::VNStep> > sequence;
    if( !ReadArgs::trace_quiet )
        fprintf(stderr, "Building patterns\n"); 
    if( ReadArgs::documentation_graphs )
        BuildDocSequence( &sequence );
    else
        BuildDefaultSequence( &sequence );    
        
    // Maybe we want to stop after buolding the steps
    if( Inferno::ShouldIQuitAfter(Progress::BUILDING_STEPS) )
        return EXIT_SUCCESS;    

    // No, so create VNSequence and Inferno instances and run it:
    // VNSequence contains the algrithms.
    // Inferno is just a harness that supports various execution 
    // scenarios based on command line args.
    auto vn_sequence = make_shared<VN::VNSequence>( sequence );
    Inferno inferno( vn_sequence );
    inferno.Run();
    //CSP::ReferenceSolver::DumpGSV();
    return EXIT_SUCCESS;
}

// TODO Consider multi-terminus Stuff and multi-root (StarStuff)

