
#include "read_args.hpp"
#include <string>

std::string ReadArgs::infile;
bool ReadArgs::trace;

void ReadArgs::Read( int argc, char *argv[] )
{
    int i=1;
    
    trace = false;
    while( i<argc )
    {
        if( argv[i]==std::string("-i") && argc>i+1 )
        {
            infile = argv[++i];
        }
        else if( argv[i]==std::string("-t") )
        {
            trace = true;
        }
        else 
        {
            fprintf(stderr, "Usage:\n%s [-t] -i <infile>\n", argv[0]);
            exit(1);
        }
        i++;    
    }    
}
