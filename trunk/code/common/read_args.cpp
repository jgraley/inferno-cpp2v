
#include "read_args.hpp"
#include <string>

std::string infile;

void read_args( int argc, char *argv[] )
{
    int i=1;
    while( i<argc )
    {
        if( argv[i]==std::string("-i") && argc>i+1 )
        {
            infile = argv[++i];
        }
        else 
        {
            fprintf(stderr, "Usage:\n%s -i <infile>\n", argv[0]);
            exit(1);
        }
        i++;    
    }    
}
