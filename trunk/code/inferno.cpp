
#include "tree/tree.hpp"
#include "common/refcount.hpp"
#include "parse/parse.hpp"  
#include "render/render.hpp"
#include "common/read_args.hpp"

int main( int argc, char *argv[] )
{
    // detects static RCTarget objects (which are bad, because we might do: delete &static_variable_oh_noes_one_one_one)
    RCTarget::Start(); 

    // Check the command line arguments 
    ReadArgs::Read( argc, argv );

    RCPtr<Program> program = new Program;  

    Parse p(ReadArgs::infile);    
    Render r;
    
    Pass *parse = &p;
    Pass *render = &r;
    (*parse)( program );
    (*render)( program );
        
    // cancel asserts and automatic indirection through RCPtr
    RCTarget::Finished(); 
}
