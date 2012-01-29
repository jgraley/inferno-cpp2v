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
#include "steps/for_to_while.hpp"
#include "steps/slave_test.hpp"
#include "steps/lower_control_flow.hpp"
#include "steps/clean_up.hpp"
#include "steps/state_out.hpp"
#include "steps/systemc_detection.hpp"
#include "steps/to_sc_method.hpp"

using namespace Steps;

void SelfTest();

    // Build a vector of transformations, in the order that we will run them
    // (ordered by hand for now, until the auto sequencer is ready)
void build_sequence( vector< shared_ptr<Transformation> > *sequence )
{
    ASSERT( sequence );
    //
    sequence->push_back( shared_ptr<Transformation>( new DetectAllSCTypes ) );

    //sequence->push_back( shared_ptr<Transformation>( new GenerateImplicitCasts ) );        
    
    sequence->push_back( shared_ptr<Transformation>( new BreakToGoto ) ); 
    sequence->push_back( shared_ptr<Transformation>( new SwitchToIfGoto ) ); 
    sequence->push_back( shared_ptr<Transformation>( new ForToWhile ) ); 
    sequence->push_back( shared_ptr<Transformation>( new WhileToDo ) ); 
    sequence->push_back( shared_ptr<Transformation>( new IfToIfGoto ) ); 
    sequence->push_back( shared_ptr<Transformation>( new DoToIfGoto ) ); 

    sequence->push_back( shared_ptr<Transformation>( new SplitInstanceDeclarations ) ); 
    sequence->push_back( shared_ptr<Transformation>( new MoveInstanceDeclarations ) ); 
    sequence->push_back( shared_ptr<Transformation>( new ExplicitiseReturn ) );
    
    sequence->push_back( shared_ptr<Transformation>( new CompactGotos ) ); // maybe put these after the label cleanups
    sequence->push_back( shared_ptr<Transformation>( new CompactGotosFinal ) );
    for( int i=0; i<2; i++ )
    {
        sequence->push_back( shared_ptr<Transformation>( new CleanupCompoundMulti ) ); 
        sequence->push_back( shared_ptr<Transformation>( new CleanupCompoundSingle ) ); 
        sequence->push_back( shared_ptr<Transformation>( new CleanupNop ) ); 
    }        
        
    sequence->push_back( shared_ptr<Transformation>( new UseTempForReturnValue ) );
    //...
    sequence->push_back( shared_ptr<Transformation>( new AddLinkAddress ) );
    sequence->push_back( shared_ptr<Transformation>( new GenerateStacks ) );
    sequence->push_back( shared_ptr<Transformation>( new MergeFunctions ) );

    // Ineffectual gotos, unused and duplicate labels result from compound tidy-up after construct lowering, but if not 
    // removed before AddGotoBeforeLabel, they will generate spurious states. We also remove dead code which can be exposed by
    // removal of unused labels - we must repeat because dead code removal can generate unused labels.
    for( int i=0; i<2; i++ )
    {
        sequence->push_back( shared_ptr<Transformation>( new CleanupUnusedLabels ) ); 
        sequence->push_back( shared_ptr<Transformation>( new CleanupDuplicateLabels ) );
        sequence->push_back( shared_ptr<Transformation>( new CleanupIneffectualLabels ) ); 
        sequence->push_back( shared_ptr<Transformation>( new CleanUpDeadCode ) ); 
    }
    sequence->push_back( shared_ptr<Transformation>( new GotoAfterWait ) );     
    sequence->push_back( shared_ptr<Transformation>( new AddGotoBeforeLabel ) );         
    sequence->push_back( shared_ptr<Transformation>( new EnsureBootstrap ) );            

    sequence->push_back( shared_ptr<Transformation>( new CleanupCompoundMulti ) );
    sequence->push_back( shared_ptr<Transformation>( new AddStateLabelVar ) ); 
    sequence->push_back( shared_ptr<Transformation>( new CleanupCompoundMulti ) ); 
            
    sequence->push_back( shared_ptr<Transformation>( new EnsureSuperLoop ) );
    sequence->push_back( shared_ptr<Transformation>( new MakeFallThroughMachine ) ); 
    sequence->push_back( shared_ptr<Transformation>( new MoveInitIntoSuperLoop ) );
    sequence->push_back( shared_ptr<Transformation>( new AddYieldFlag ) );
    sequence->push_back( shared_ptr<Transformation>( new AddInferredYield ) ); // now yielding in every iteration of superloop
    sequence->push_back( shared_ptr<Transformation>( new LoopRotation ) );

    sequence->push_back( shared_ptr<Transformation>( new VarsToModule ) );
    sequence->push_back( shared_ptr<Transformation>( new DeclsToModule ) );
    sequence->push_back( shared_ptr<Transformation>( new ThreadToMethod ) );

    for( int i=0; i<2; i++ )
    {
        sequence->push_back( shared_ptr<Transformation>( new CleanupUnusedLabels ) ); 
        sequence->push_back( shared_ptr<Transformation>( new CleanupDuplicateLabels ) );
        sequence->push_back( shared_ptr<Transformation>( new CleanupIneffectualLabels ) ); 
        sequence->push_back( shared_ptr<Transformation>( new CleanUpDeadCode ) ); 
    }
}


int main( int argc, char *argv[] )
{
    // Check the command line arguments 
    ReadArgs( argc, argv );
    Tracer::Enable( ReadArgs::trace );

    // Do self-tests (unit tests) if requested
    if( ReadArgs::selftest )
        SelfTest();
    
    // get the transformations, in sequence, in a vector
    vector< shared_ptr<Transformation> > sequence;
    build_sequence( &sequence );
    
    // If a pattern graph was requested, generate it now
    if( ReadArgs::pattern_graph != -1 )
    {
        ASSERT( ReadArgs::pattern_graph >= 0 )("Negative step number is silly\n");
        ASSERT( ReadArgs::pattern_graph < sequence.size() )("There are only %d steps at present\n", sequence.size() );
        Graph()( sequence[ReadArgs::pattern_graph].get() );
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
        fprintf(stderr, "Step %d: %s\n", ReadArgs::runonlystep, string( *t ).c_str() ); 
        TRACE("Step %d: %s\n", ReadArgs::runonlystep, string( *t ).c_str() ); // TODO trace should print to stderr too
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
            fprintf(stderr, "Step %d: %s\n", i, string( *t ).c_str() ); 
            Tracer::Enable( ReadArgs::trace && (!ReadArgs::quitenable || ReadArgs::quitafter==0) ); 
            if( (!ReadArgs::quitenable || ReadArgs::quitafter==0) )
                CompareReplace::SetMaxReps( ReadArgs::repetitions, ReadArgs::rep_error );
            else
                CompareReplace::SetMaxReps( 100, true );
            HitCount::instance.SetStep(i);
            (*t)( &program );
            i++;
        }
    }
        
    // Output either C source code or a graph, as requested
    Tracer::Enable( ReadArgs::trace );
    if(ReadArgs::intermediate_graph)
        Graph()( &program );    
    else    
        Render()(&program );     
        
    if( ReadArgs::trace_hits )
        HitCount::instance.Dump();    
    
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
