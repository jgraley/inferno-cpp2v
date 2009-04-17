
#include "read_args.hpp"
#include <string>

std::string ReadArgs::infile;
std::string ReadArgs::outfile;
bool ReadArgs::graph;
bool ReadArgs::trace;
bool ReadArgs::selftest;

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
