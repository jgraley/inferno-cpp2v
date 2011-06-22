
#include "tree/tree.hpp"
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
#include "steps/use_temps_for_params_return.hpp"
#include "steps/generate_stacks.hpp"
#include "steps/for_to_while.hpp"
#include "steps/slave_test.hpp"
#include "steps/lower_control_flow.hpp"
#include "steps/clean_up.hpp"
#include "steps/state_out.hpp"

void SelfTest();

void build_sequence( vector< shared_ptr<Transformation> > *sequence )
{
    ASSERT( sequence );
    // Build a vector of transformations, in the order that we will run them
    // (ordered by hand for now, until the auto sequencer is ready)
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
    
    sequence->push_back( shared_ptr<Transformation>( new CompactGotos ) );
    sequence->push_back( shared_ptr<Transformation>( new CompactGotosFinal ) );
    for( int i=0; i<2; i++ )
    {
        //sequence->push_back( shared_ptr<Transformation>( new CleanupCompoundMulti ) ); 
        sequence->push_back( shared_ptr<Transformation>( new CleanupCompoundSingle ) ); 
        sequence->push_back( shared_ptr<Transformation>( new CleanupNop ) ); 
        sequence->push_back( shared_ptr<Transformation>( new CleanupDuplicateLabels ) ); 
        //sequence->push_back( shared_ptr<Transformation>( new CleanupIneffectualGoto ) ); // TODO change this to only act when no other usages of the label (ie we are not marging code paths)
        sequence->push_back( shared_ptr<Transformation>( new CleanupUnusedLabels ) ); 
    }        
    sequence->push_back( shared_ptr<Transformation>( new EnsureBootstrap ) );
    sequence->push_back( shared_ptr<Transformation>( new AddStateLabelVar ) );
   
    /*    
    sequence->push_back( shared_ptr<Transformation>( new UseTempsForParamsReturn ) );     
    sequence->push_back( shared_ptr<Transformation>( new GenerateStacks ) ); 
        */
    // These clean-up steps run a few times, because they need to clean up after each other
    for( int i=0; i<3; i++ )
    {
        sequence->push_back( shared_ptr<Transformation>( new CleanupDuplicateLabels ) ); 
        sequence->push_back( shared_ptr<Transformation>( new CleanupCompoundMulti ) ); 
        sequence->push_back( shared_ptr<Transformation>( new CleanupCompoundSingle ) );
        sequence->push_back( shared_ptr<Transformation>( new CleanupNop ) );          
        //sequence->push_back( shared_ptr<Transformation>( new CleanupIneffectualGoto ) ); 
        sequence->push_back( shared_ptr<Transformation>( new CleanupUnusedLabels ) ); 
    }        

    sequence->push_back( shared_ptr<Transformation>( new EnsureSuperLoop ) );
    sequence->push_back( shared_ptr<Transformation>( new ShareGotos ) );
    sequence->push_back( shared_ptr<Transformation>( new InsertSwitch ) ); 
    sequence->push_back( shared_ptr<Transformation>( new InferBreak ) ); 
}


int main( int argc, char *argv[] )
{
    // Check the command line arguments 
    ReadArgs( argc, argv );

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
 
    // Apply the transformation steps in order, but quit early if requested to
    int i=0;
    FOREACH( shared_ptr<Transformation> t, sequence )
    {
        if( ReadArgs::quitafter-- == 0 )
            break;
        fprintf(stderr, "Step %d: %s\n", i, string( *t ).c_str() ); // TODO trace should print to stderr too    
        TRACE("Step %d: %s\n", i, string( *t ).c_str() ); // TODO trace should print to stderr too
        HitCount::instance.SetStep(i);
        (*t)( &program );
        i++;
    }
    
    // Output either C source code or a graph, as requested
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
// TODO Docs for node interface. 
// TODO Improve comments in tree.h
// TODO Star restriction pattern (eg for dead code elimination want Star(NotMatch(Label)))
// TODO Consider parent restriction for usages - but tricky case: int x, int y=x. Decl for y points to x (as init expr) and y (as ident). Need to specify parent node *and* which TreePtr in the parent is to be excluded.
// TODO Consider multi-terminus Stuff and multi-root (StarStuff)
