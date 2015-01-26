#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "parse/parse.hpp"  
#include "render/render.hpp"
#include "render/graph.hpp"
#include "common/read_args.hpp"
#include "common/hit_count.hpp"
#include "helpers/walk.hpp"
#include "sr/search_replace.hpp"
#include "sr/soft_patterns.hpp"
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
    // Build a vector of transformations, in the order that we will run them
    // (ordered by hand for now, until the auto sequencer is ready)
void BuildSequence( vector< shared_ptr<Transformation> > *sequence )
{
    ASSERT( sequence );
    // SystemC detection, converts implicit SystemC to explicit. Always at the top
    // because we never want to process implicit SystemC.
    sequence->push_back( shared_ptr<Transformation>( new DetectAllSCTypes ) );

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
        
        sequence->push_back( shared_ptr<Transformation>( new LogicalAndToIf ) ); 
        sequence->push_back( shared_ptr<Transformation>( new LogicalOrToIf ) ); 
        sequence->push_back( shared_ptr<Transformation>( new MultiplexorToIf ) ); 
        sequence->push_back( shared_ptr<Transformation>( new SwitchToIfGoto ) );
        sequence->push_back( shared_ptr<Transformation>( new SplitInstanceDeclarations ) );  
        sequence->push_back( shared_ptr<Transformation>( new IfToIfGoto ) ); 
        // All remaining uncomables at the top level and in SUSP style
    }    
    { // Initial treatment of gotos and labels
        sequence->push_back( shared_ptr<Transformation>( new NormaliseConditionalGotos ) );
        sequence->push_back( shared_ptr<Transformation>( new CompactGotos ) ); // maybe put these after the label cleanups
    }        
    { // big round of cleaning up
        sequence->push_back( shared_ptr<Transformation>( new ReduceVoidCompoundExpression ) ); 
        //for( int i=0; i<2; i++ )
        // Ineffectual gotos, unused and duplicate labels result from compound tidy-up after construct lowering, but if not 
        // removed before AddGotoBeforeLabel, they will generate spurious states. We also remove dead code which can be exposed by
        // removal of unused labels - we must repeat because dead code removal can generate unused labels.
        sequence->push_back( shared_ptr<Transformation>( new CleanupCompoundExpression ) ); // TODO only safe in SSP, so don't call this a cleanup!
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
            sequence->push_back( shared_ptr<Transformation>( new SwapSubscriptMultiplex ) );        
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


int main( int argc, char *argv[] )
{
    // Check the command line arguments 
    ReadArgs( argc, argv );
    HitCount::instance.Check();
    Tracer::Enable( ReadArgs::trace );
    HitCount::instance.SetStep(-1);
    HitCount::Enable( ReadArgs::trace_hits );

    // Do self-tests (unit tests) if requested
    if( ReadArgs::selftest )
        SelfTest();
    
    // Build documentation graphs if requested
    if( ReadArgs::documentation_graphs )
        GenerateDocumentationGraphs();
    
    // get the transformations, in sequence, in a vector
    vector< shared_ptr<Transformation> > sequence;
    BuildSequence( &sequence );
    
    // If a pattern graph was requested, generate it now
    if( ReadArgs::pattern_graph != -1 )
    {
        ASSERT( ReadArgs::pattern_graph >= 0 )("Negative step number is silly\n");
        ASSERT( ReadArgs::pattern_graph < sequence.size() )("There are only %d steps at present\n", sequence.size() );
        Graph g( ReadArgs::outfile );
        g( sequence[ReadArgs::pattern_graph].get() );
    }        
    
    // If there was no input program then there's nothing more to do
    if( ReadArgs::infile.empty() )
        return 0;

    // Parse the input program
    TreePtr<Node> program = TreePtr<Node>();
    Parse p(ReadArgs::infile);
    p( program, &program );
 
    if( ReadArgs::runonlyenable )
    {
        // Apply only the transformation requested
        shared_ptr<Transformation> t = sequence[ReadArgs::runonlystep];
        if( !ReadArgs::trace_quiet )
            fprintf(stderr, "%s step %d: %s\n", ReadArgs::infile.c_str(), ReadArgs::runonlystep, string( *t ).c_str() ); 
        HitCount::instance.SetStep(ReadArgs::runonlystep);
        CompareReplace::SetMaxReps( ReadArgs::repetitions, ReadArgs::rep_error );
        (*t)( &program );
    }
    else
    {
        // Apply the transformation steps in order, but quit early if requested to
        int i=0;
        FOREACH( shared_ptr<Transformation> t, sequence )
        {
            if( ReadArgs::quitafter-- == 0 )
                break;
            if( !ReadArgs::trace_quiet )
                fprintf(stderr, "%s step %d: %s\n", ReadArgs::infile.c_str(), i, string( *t ).c_str() ); 
            Tracer::Enable( ReadArgs::trace && (!ReadArgs::quitenable || ReadArgs::quitafter==0) ); 
            HitCount::Enable( ReadArgs::trace_hits && (!ReadArgs::quitenable || ReadArgs::quitafter==0) ); 
            if( (!ReadArgs::quitenable || ReadArgs::quitafter==0) )
                CompareReplace::SetMaxReps( ReadArgs::repetitions, ReadArgs::rep_error );
            else
                CompareReplace::SetMaxReps( 100, true );
            HitCount::instance.SetStep(i);
            (*t)( &program );
            if( ReadArgs::output_all )
            {
                Render r( ReadArgs::outfile+SSPrintf("_%03d.cpp", i) );
                r( &program );     
                // TODO prove that the graph renderer does not modify the tree before enabling
                //Graph g( ReadArgs::outfile+SSPrintf("_%03d.dot", i) );
                //g( &program );    
            }
                
            i++;
        }
    }
        
    // Output either C source code or a graph, as requested
    Tracer::Enable( ReadArgs::trace );
    HitCount::Enable( ReadArgs::trace_hits );
    HitCount::instance.SetStep(-1);
    if( ReadArgs::trace_hits )
        HitCount::instance.Dump();    
    else if( ReadArgs::intermediate_graph && !ReadArgs::output_all )
    {
        Graph g( ReadArgs::outfile );
        g( &program );    
    }
    else if( !ReadArgs::output_all )   
    {
        Render r( ReadArgs::outfile );
        r( &program );     
    }
            
    return 0;
}

void SelfTest()

{
    SearchReplace::Test();
    CommonTest();
    GenericsTest();
}

// TODO Split Compare out of CompareReplace. 
// TODO Make Filter a functor. 
// TODO Consider merging Filter into Transformation.
// TODO Produce base class for builder nodes: TransformTo?
// TODO Consider multi-terminus Stuff and multi-root (StarStuff)

