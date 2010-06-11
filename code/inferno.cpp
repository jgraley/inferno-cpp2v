
#include "tree/tree.hpp"
#include "common/refcount.hpp"
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

void SelfTest();

int main( int argc, char *argv[] )
{
    // Check the command line arguments 
    ReadArgs::Read( argc, argv );

    if( ReadArgs::selftest )
        SelfTest();

    if( ReadArgs::infile.empty() )
        return 0;

    SharedPtr<Node> program = SharedPtr<Node>();
    Parse p(ReadArgs::infile);
    p( program, &program );
              
    if( --ReadArgs::quitafter >= 0 )
    {
    	ForToWhile()(program, &program);
    }

    if( --ReadArgs::quitafter >= 0 )
    {
    	GenerateImplicitCasts()(program, &program);
    }

    if( --ReadArgs::quitafter >= 0 )
    {
        SplitInstanceDeclarations()(program, &program);
    }

    if( --ReadArgs::quitafter >= 0 )
    {
        UseTempsForParamsReturn()(program, &program);
    }

    if( --ReadArgs::quitafter >= 0 )
    {
    	GenerateStacks()(program, &program);
    }

    if(ReadArgs::intermediate_graph)
    {
        Graph g;
        g( program, program );
    }
    else
    {
        Render r;
        r( program, program );
    }    
    
    return 0;
}

void SelfTest()
{
    SearchReplace::Test();
    CommonTest();
    GenericsTest();
}

