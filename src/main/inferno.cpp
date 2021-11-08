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

using namespace Steps;

void SelfTest();

struct StepPlan
{
    shared_ptr<Transformation> tx;
    int step_index;
    bool allow_trace;
    bool allow_hits;
    bool allow_reps;
};


// Build a vector of transformations, in the order that we will run them
// (ordered by hand for now, until the auto sequencer is ready)
void BuildSequence( vector< shared_ptr<Transformation> > *sequence )
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

    //sequence->push_back( shared_ptr<Transformation>( new SlaveTest2 ) ); 
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


void MaybeGeneratePatternGraphs( const vector<StepPlan> &steps_plan )
{
    if( !ReadArgs::pattern_graph_name.empty() || ReadArgs::pattern_graph_index != -1 )
    {
        if( ReadArgs::pattern_graph_name.back()=='/' )
        {
            string dir = ReadArgs::pattern_graph_name;
            for( const StepPlan &sp : steps_plan )
            {
				Progress(Progress::RENDERING, sp.step_index).SetAsCurrent();
                string filepath = SSPrintf("%s%03d-%s.dot", dir.c_str(), sp.step_index, sp.tx->GetName().c_str());                                                       
                Graph g( filepath, sp.tx->GetName() );
                GenerateGraphRegions( g, sp.tx );
            }
        }
        else
        {
            StepPlan my_sp;
            if( ReadArgs::pattern_graph_name.empty() )
            {
                ASSERT( ReadArgs::pattern_graph_index >= 0 )("Negative step number is silly\n");
                ASSERT( ReadArgs::pattern_graph_index < steps_plan.size() )("There are only %d steps at present\n", steps_plan.size() );
                my_sp = steps_plan[ReadArgs::pattern_graph_index];
            }
            else
            {
                for( const StepPlan &sp : steps_plan )
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
                    for( const StepPlan &sp : steps_plan )
                        fprintf(stderr, "%03d-%s\n", sp.step_index, sp.tx->GetName().c_str() );
                    ASSERT(false);
                }
            }
			Progress(Progress::RENDERING, my_sp.step_index).SetAsCurrent();
            Graph g( ReadArgs::outfile, my_sp.tx->GetName() );
            GenerateGraphRegions( g, my_sp.tx );
        }
    }        
}


bool ShouldIQuit()
{
    if( ReadArgs::quitafter )
    {
        if( ReadArgs::quitafter_progress.GetStage()==Progress::GetCurrentStage().GetStage() )
        {
            //FTRACE("Stopping after ")(ReadArgs::quitafter_progress)("\n");
            return true;
        }
    }
    return false;
}
    

int main( int argc, char *argv[] )
{
    // Check the command line arguments 
    ReadArgs( argc, argv );
    HitCount::instance.Check();
    Tracer::Enable( ReadArgs::trace );
    Progress(Progress::BUILDING_STEPS).SetAsCurrent();    
    HitCount::Enable( ReadArgs::trace_hits );
    int i;

    // Do self-tests (unit tests) if requested
    if( ReadArgs::selftest )
        SelfTest();
    
    // Build documentation graphs if requested
    if( ReadArgs::documentation_graphs )
        GenerateDocumentationGraphs();
    
    // Build the sequence of steps
    if( !ReadArgs::trace_quiet )
		fprintf(stderr, "Building patterns\n"); 
    vector< shared_ptr<Transformation> > sequence;
    BuildSequence( &sequence );
    if( ShouldIQuit() )
        exit(0);    

    // Start a steps plan
    vector<StepPlan> steps_plan;
    for( int i=0; i<sequence.size(); i++ )
        steps_plan.push_back( { sequence[i], i, ReadArgs::trace, ReadArgs::trace_hits, true } );        
    
    // If we're to run only one step, restrict all stepped stages
    if( ReadArgs::runonlyenable )
        steps_plan = { steps_plan[ReadArgs::runonlystep] };

    // If we're to quit after a particular step, restrict all stepped stages
    if( ReadArgs::quitafter )
    {
        if( ReadArgs::quitafter_progress.GetStep() != Progress::NO_STEP )
            steps_plan.resize(ReadArgs::quitafter_progress.GetStep() + 1);
        for( int i=0; i<steps_plan.size()-1; i++ )
            steps_plan[i].allow_trace = steps_plan[i].allow_hits = steps_plan[i].allow_reps = false;
    }

    if( !ReadArgs::graph_trace )
        MaybeGeneratePatternGraphs( steps_plan );
                
    // Pattern transformations
    if( !ReadArgs::trace_quiet )
		fprintf(stderr, "Pattern transforming\n"); 
    for( const StepPlan &sp : steps_plan )
    {
        Progress(Progress::PATTERN_TRANS, sp.step_index).SetAsCurrent();
        Tracer::Enable( sp.allow_trace ); 
        if( auto pvnt = dynamic_pointer_cast<VNTransformation>(sp.tx) )
            pvnt->PatternTransformations();
        else
            ASSERTFAIL("Unknown transformation");
    }        
        
    // Planning part one
    if( !ReadArgs::trace_quiet )
		fprintf(stderr, "Planning stage one\n"); 
    for( const StepPlan &sp : steps_plan )
    {
        Progress(Progress::PLANNING_ONE, sp.step_index).SetAsCurrent();
        Tracer::Enable( sp.allow_trace ); 
        if( auto pvnt = dynamic_pointer_cast<VNTransformation>(sp.tx) )
            pvnt->PlanningStageOne();
        else
            ASSERTFAIL("Unknown transformation");
    }

    if( ShouldIQuit() )
        exit(0);
        
    // Planning part two
    if( !ReadArgs::trace_quiet )
		fprintf(stderr, "Planning stage two\n"); 
    for( const StepPlan &sp : steps_plan )
    {
        Progress(Progress::PLANNING_TWO, sp.step_index).SetAsCurrent();
        Tracer::Enable( sp.allow_trace ); 
        if( auto pvnt = dynamic_pointer_cast<VNTransformation>(sp.tx) )
            pvnt->PlanningStageTwo();
        else
            ASSERTFAIL("Unknown transformation");
    }
           
    if( ShouldIQuit() )
        exit(0);

    // Planning part three
    if( !ReadArgs::trace_quiet )
		fprintf(stderr, "Planning stage three\n"); 
    for( const StepPlan &sp : steps_plan )
    {
        Progress(Progress::PLANNING_THREE, sp.step_index).SetAsCurrent();
        Tracer::Enable( sp.allow_trace ); 
        if( auto pvnt = dynamic_pointer_cast<VNTransformation>(sp.tx) )
            pvnt->PlanningStageThree();
        else
            ASSERTFAIL("Unknown transformation");
    }
       
    // If a pattern graph was requested, generate it now. We need the
    // agents to have been configured (planning stage 2)
    if( ReadArgs::graph_trace )
        MaybeGeneratePatternGraphs( steps_plan );

    if( ShouldIQuit() )
        exit(0);

    // If there was no input program then there's nothing more to do
    if( ReadArgs::infile.empty() )
        return 0;
    
    // Parse the input program
    Progress(Progress::PARSING).SetAsCurrent();   
    Tracer::Enable( ReadArgs::trace );
    if( !ReadArgs::trace_quiet )
		fprintf(stderr, "Parsing input %s\n", ReadArgs::infile.c_str()); 
    TreePtr<Node> program = TreePtr<Node>();
    {
        //Tracer::RAIIEnable silencer( false ); // make parse be quiet
        Parse p(ReadArgs::infile);
        p( program, &program );
    }
            
    if( !ShouldIQuit() ) // Now input has been parsed, we always want to render even if quitting early.
    {
        // Apply the transformation steps in order, but quit early if requested to
        for( const StepPlan &sp : steps_plan )
        {
            Progress(Progress::TRANSFORMING, sp.step_index).SetAsCurrent();
                       
            bool allow = !ReadArgs::quitafter || ReadArgs::quitafter_progress==Progress::GetCurrent();
            if( !ReadArgs::trace_quiet )
                fprintf(stderr, "%s at T%03d-%s\n", ReadArgs::infile.c_str(), sp.step_index, sp.tx->GetName().c_str() ); 
            Tracer::Enable( sp.allow_trace ); 
            HitCount::Enable( sp.allow_hits ); 
            if( sp.allow_reps )
                VNTransformation::SetMaxReps( ReadArgs::repetitions, ReadArgs::rep_error );
            else
                VNTransformation::SetMaxReps( 100, true );
            if( allow )
                sp.tx->SetStopAfter(ReadArgs::quitafter_counts, 0);
            (*sp.tx)( &program );
            if( ReadArgs::output_all )
            {
                Render r( ReadArgs::outfile+SSPrintf("_%03d.cpp", sp.step_index) );
                r( &program );     
                Graph g( ReadArgs::outfile+SSPrintf("_%03d.dot", sp.step_index), 
                         ReadArgs::outfile+SSPrintf(" after T%03d-%s", sp.step_index, sp.tx->GetName().c_str()) );
                g( &program );    
            }           
        }
    }
        
    // Output either C source code or a graph, as requested
    Tracer::Enable( ReadArgs::trace );
    HitCount::Enable( ReadArgs::trace_hits );
    Progress(Progress::RENDERING).SetAsCurrent();
    if( ReadArgs::trace_hits )
        HitCount::instance.Dump();    
    else if( ReadArgs::intermediate_graph && !ReadArgs::output_all )
    {
        if( !ReadArgs::trace_quiet )
			fprintf(stderr, "Rendering to graph\n"); 
        Tracer::RAIIEnable silencer( false ); // make grapher be quiet
        Graph g( ReadArgs::outfile, ReadArgs::outfile );
        g( &program );    
    }
    else if( !ReadArgs::output_all )   
    {
        if( !ReadArgs::trace_quiet )
			fprintf(stderr, "Rendering to code\n"); 
        Tracer::RAIIEnable silencer( false ); // make render be quiet
        Render r( ReadArgs::outfile );
        r( &program );     
    }
            
    return 0;
}

void SelfTest()

{
    CommonTest();
    GenericsTest();
}

// TODO Make Filter a functor. 
// TODO Consider merging Filter into Transformation.
// TODO Consider multi-terminus Stuff and multi-root (StarStuff)

