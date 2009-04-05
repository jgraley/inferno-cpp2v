
#include "tree/tree.hpp"
#include "common/refcount.hpp"
#include "parse/parse.hpp"  
#include "render/render.hpp"
#include "render/graph.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/search_replace.hpp"

int main( int argc, char *argv[] )
{
    // Check the command line arguments 
    ReadArgs::Read( argc, argv );

    shared_ptr<Program> program(new Program);  

    Parse p(ReadArgs::infile);        
    p( program );
 
    //WalkAndPrint(program);       
         
    Graph r;
    r( program );
}
