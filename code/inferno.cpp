
#include "tree/tree.hpp"
#include "common/refcount.hpp"
#include "parse/parse.hpp"  
#include "render/render.hpp"
#include "render/graph.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/search_replace.hpp"

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
          
    shared_ptr<Integer> i( new Integer(16) );
    shared_ptr<Integral> v( new Integral );
    v->width = i;
    bool found = !!SearchReplace::Search( program, v );       
    printf(found?"YES!!!!\n":"NO!!!!!!\n");
          
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
}
