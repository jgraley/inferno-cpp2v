
#include "tree/tree.hpp"
#include "common/refcount.hpp"
#include "parse/parse.hpp"  
#include "render/render.hpp"
#include "common/read_args.hpp"

int main( int argc, char *argv[] )
{
    RCTarget::Start();

    read_args( argc, argv );

    RCPtr<Program> program = new Program();  

    Parse p;    
    p( program, infile );
    
    Render r;
    std::string ss = r( program );
    
    printf( "%s", ss.c_str() );   
        
    RCTarget::Finished();
}
