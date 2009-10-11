
#include "read_args.hpp"
#include <string>
#include <stdlib.h>

std::string ReadArgs::infile;
std::string ReadArgs::outfile;
bool ReadArgs::graph;
bool ReadArgs::trace;
bool ReadArgs::selftest;
unsigned ReadArgs::quitafter;

void ReadArgs::Usage()
{
    fprintf(stderr, "Usage:\ninferno [-t] [-s] [-g] [-i <infile>] [-o <outfile>]\n"
                    "One of -i or -s required\n" );
    exit(1);
}

void ReadArgs::Read( int argc, char *argv[] )
{ 
    int i=1;
    trace = false;
    graph = false;
    selftest = false;
    quitafter = 0xffffffff;
    while( i<argc )
    {
        if( argv[i]==std::string("-i") && argc>i+1 )
        {
            infile = argv[++i];
        }
        else if( argv[i]==std::string("-o") && argc>i+1 )
        {
            outfile = argv[++i];
        }
        else if( argv[i]==std::string("-t") )
        {
            trace = true;
        }
        else if( argv[i]==std::string("-g") )
        {
            graph = true;
        }
        else if( argv[i]==std::string("-s") )
        {
            selftest = true;
        }
        else if( argv[i]==std::string("-q") )
        {
        	quitafter = strtoul( argv[++i], NULL, 10 );
        }
        else 
        {
            Usage();
        }
        i++;    
    }    
    
    // infile or selftest is always required
    if( infile.empty() && !selftest )
        Usage();
}
