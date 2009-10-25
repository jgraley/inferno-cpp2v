
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

void SelfTest();

int main( int argc, char *argv[] )
{
    // Check the command line arguments 
    ReadArgs::Read( argc, argv );

    if( ReadArgs::selftest )
        SelfTest();

    if( ReadArgs::infile.empty() )
        return 0;

    shared_ptr<Program> program(new Program);  

    Parse p(ReadArgs::infile);        
    p( program );
              
    Validate()(program);                

    if( --ReadArgs::quitafter >= 0 )
    {
        SplitInstanceDeclarations()(program);
        Validate()(program);
    }

    if( --ReadArgs::quitafter >= 0 )
    {
    	GenerateImplicitCasts()(program);
        Validate()(program);
    }

    if( --ReadArgs::quitafter >= 0 )
    {
    	HackUpIfs()(program);
        Validate()(program);
    }

    if(ReadArgs::graph)
    {
        Graph g;
        g( program );
    }
    else
    {
        Render r;
        r( program );
    }    
    
    return 0;
}

void SelfTest()
{
    SearchReplace::Test();
    CommonTest();
    GenericsTest();
}

