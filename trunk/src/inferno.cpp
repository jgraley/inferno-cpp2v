
#include "tree/tree.hpp"
#include "parse/parse.hpp"  
#include "render/render.hpp"
#include "render/graph.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/search_replace.hpp"
#include "helpers/soft_patterns.hpp"
#include "helpers/validate.hpp"
#include "steps/split_instance_declarations.hpp"
#include "steps/generate_implicit_casts.hpp"
#include "steps/use_temps_for_params_return.hpp"
#include "steps/generate_stacks.hpp"
#include "steps/for_to_while.hpp"
#include "steps/slave_test.hpp"
#include "steps/lower_control_flow.hpp"
#include "steps/clean_up.hpp"

void SelfTest();


void build_sequence( vector< shared_ptr<Transformation> > *sequence )
{
    ASSERT( sequence );
    // Build a vector of transformations, in the order that we will run them
    // (ordered by hand for now, until the suto sequencer is ready)
    sequence->push_back( shared_ptr<Transformation>( new BreakToGoto ) ); 
    sequence->push_back( shared_ptr<Transformation>( new SwitchToIfGoto ) ); 
    sequence->push_back( shared_ptr<Transformation>( new ForToWhile ) ); 
    sequence->push_back( shared_ptr<Transformation>( new WhileToDo ) ); 
    sequence->push_back( shared_ptr<Transformation>( new IfToIfGoto ) ); 
    sequence->push_back( shared_ptr<Transformation>( new DoToIfGoto ) ); 
    sequence->push_back( shared_ptr<Transformation>( new GenerateImplicitCasts ) ); 
    sequence->push_back( shared_ptr<Transformation>( new SplitInstanceDeclarations ) ); 
    sequence->push_back( shared_ptr<Transformation>( new MoveInstanceDeclarations ) ); 
    sequence->push_back( shared_ptr<Transformation>( new UseTempsForParamsReturn ) ); 
    sequence->push_back( shared_ptr<Transformation>( new GenerateStacks ) ); 
    
    // These clean-up steps run a few times, because they need to clean up after themselves
    for( int i=0; i<3; i++ )
    {
        sequence->push_back( shared_ptr<Transformation>( new CleanupCompoundMulti ) ); 
        sequence->push_back( shared_ptr<Transformation>( new CleanupCompoundSingle ) ); 
        sequence->push_back( shared_ptr<Transformation>( new CleanupNop ) ); 
        sequence->push_back( shared_ptr<Transformation>( new CleanupDuplicateLabels ) ); 
        sequence->push_back( shared_ptr<Transformation>( new CleanupIneffectualGoto ) ); 
        sequence->push_back( shared_ptr<Transformation>( new CleanupUnusedLabels ) ); 
    }        
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
    FOREACH( shared_ptr<Transformation> t, sequence )
    {
        if( ReadArgs::quitafter-- == 0 )
            break;
        (*t)( &program );
    }
    
    // Output either C source code or a graph, as requested
    if(ReadArgs::intermediate_graph)
        Graph()( &program );    
    else    
        Render()(&program );     
    
    return 0;
}

void SelfTest()
{
    SearchReplace::Test();
    CommonTest();
    GenericsTest();
}

// TODO reorg dirs: move tree-aware helpers into tree/
// split out non-tree-aware generics and Node into a new interfaces/ dir and put transformation there too
// rename helpers/ to something like tools/
// deal with TransformOf embedded in tree-aware helpers like TypeOf by undoing that and requiring steps to 
// instance a template explicitly.
// Set dependencies and include paths based on this "diamond" dependency structure
// Split Compare out of CompareReplace. MAke Filter a functor. Consider merging Filter into tRansformation.
// Produce base class for builder nodes: TransformTo?
// Docs for node interface. Improve comments in tree.h
// Star restriction pattern (eg for dead code elimination want Star(NotMatch(Label)))
// Consider parent restriction for usages
// Consider multi-terminus Stuff and multi-root (StarStuff)
