
#include "tree/tree.hpp"
#include "common/refcount.hpp"
#include "parse/parse.hpp"  
#include "render/render.hpp"
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
        
/*    Walk w( program );
    int count=0;
    while(!w.Done())
    {
        shared_ptr<Node> n = w.Get();
        for( int i=0; i<w.Depth(); i++ )
            printf(" ");
        if(n)
            printf("%s", typeid(*n).name());
        else
            printf("null");
        w.Advance(); 
        printf("\n");
        count++;
    }   
  */      
    Render r;
    r( program );
}
