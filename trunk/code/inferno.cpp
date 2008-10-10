
#include "tree/tree.hpp"
#include "common/refcount.hpp"
#include "parse/parse.hpp"  
#include "render/render.hpp"
#include "common/read_args.hpp"

int main( int argc, char *argv[] )
{
    // Check the command line arguments 
    ReadArgs::Read( argc, argv );

    shared_ptr<Program> program(new Program);  

    Parse p(ReadArgs::infile);    
    Render r;
    
    Pass *parse = &p;
    Pass *render = &r;
    (*parse)( program );
    (*render)( program );
}
