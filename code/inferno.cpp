
#include "tree/tree.hpp"
#include "common/refcount.hpp"
#include "parse/parse.hpp"  
#include "render/render.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"

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
    
/*    Object o;    
    int oi=(int)&o;
    printf("Ob=%p Id=%p Decl=%p Ex=%p Phy=%p StDecl=%p StEx=%p N=%p\n",
           0,
           (int)(Identifier *)&o - oi,
           (int)(Declaration *)&o - oi,
           (int)(Expression *)&o - oi,
           (int)(Physical *)&o - oi,
           (int)(Statement *)(Declaration *)&o - oi,
           (int)(Statement *)(Expression *)&o - oi,
           (int)(Node *)&o - oi );    
  */  
  
    Walk<Procedure> w(program);
    
    (*render)( program );
}
