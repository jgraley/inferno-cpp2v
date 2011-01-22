
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

int main( int argc, char *argv[] )
{
    // Check the command line arguments 
    ReadArgs( argc, argv );

    if( ReadArgs::selftest )
        SelfTest();

    if( ReadArgs::infile.empty() )
        return 0;

    TreePtr<Node> program = TreePtr<Node>();
    Parse p(ReadArgs::infile);
    p( program, &program );
              
    if( --ReadArgs::quitafter >= 0 )
    {
        BreakToGoto()(&program);
    }

    if( --ReadArgs::quitafter >= 0 )
    {
    	SwitchToIfGoto()(&program);
    }

    if( --ReadArgs::quitafter >= 0 )
    {
    	ForToWhile()(&program);
    }

    if( --ReadArgs::quitafter >= 0 )
    {
        WhileToDo()(&program);
    }

    if( --ReadArgs::quitafter >= 0 )
    {
        IfToIfGoto()(&program);
    }

    if( --ReadArgs::quitafter >= 0 )
    {
        DoToIfGoto()(&program);
    }

    if( --ReadArgs::quitafter >= 0 )
    {
    	GenerateImplicitCasts()(&program);
    }

    if( --ReadArgs::quitafter >= 0 )
    {
        SplitInstanceDeclarations()(&program);
    }

    if( --ReadArgs::quitafter >= 0 )
    {
        MoveInstanceDeclarations()(&program);
    }

    if( --ReadArgs::quitafter >= 0 )
    {
        UseTempsForParamsReturn()(&program);
    }

    if( --ReadArgs::quitafter >= 0 )
    {
    	GenerateStacks()(&program);
    }

  	for( int i=0; i<3; i++ ) // TODO better way of repeating steps
	{
	    if( i==0 )
			--ReadArgs::quitafter;
		if( ReadArgs::quitafter >= 0 ) 
	    {
	        CleanupCompoundMulti()(&program);
		}

	    if( i==0 )
			--ReadArgs::quitafter;
	    if( ReadArgs::quitafter >= 0 ) 
		{
	        CleanupCompoundSingle()(&program);
		}
		
	    if( i==0 )
			--ReadArgs::quitafter;
	    if( ReadArgs::quitafter >= 0 ) 
		{
	        CleanupNop()(&program);
		}
	    if( i==0 )
			--ReadArgs::quitafter;
	    if( ReadArgs::quitafter >= 0 ) 
		{
	        CleanupDuplicateLabels()(&program);
		}
	    if( i==0 )
			--ReadArgs::quitafter;
	    if( ReadArgs::quitafter >= 0 ) 
		{
	        CleanupIneffectualGoto()(&program);
		}

	    if( i==0 )
			--ReadArgs::quitafter;
	    if( ReadArgs::quitafter >= 0 ) 
		{
	        CleanupUnusedLabels()(&program);
	    }
    }

    if(ReadArgs::intermediate_graph)
    {
        Graph()( &program );
    }
    else
    {
        Render()(&program );
    }    
    
    return 0;
}

void SelfTest()
{
    SearchReplace::Test();
    CommonTest();
    GenericsTest();
}

