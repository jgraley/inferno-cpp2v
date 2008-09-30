
#include "tree/tree.hpp"
#include "common/refcount.hpp"
#include "parse/parse.hpp"  
#include "render/render.hpp"
#include "common/read_args.hpp"

int main( int argc, char *argv[] )
{
    RCTarget::Start();

    read_args( argc, argv );

    RCPtr<Program> program = new Program;  

    Parse p(infile);    
    Render r;
    
    Pass *parse = &p;
    Pass *render = &r;
    (*parse)( program );
    (*render)( program );
        
    RCTarget::Finished();
}
