
#include "read_args.hpp"
#include <string>
#include <stdlib.h>

std::string ReadArgs::exename;
std::string ReadArgs::infile;
std::string ReadArgs::outfile;
bool ReadArgs::intermediate_graph;
bool ReadArgs::pattern_graph;
bool ReadArgs::trace;
bool ReadArgs::selftest;
int ReadArgs::quitafter;
bool ReadArgs::quitenable;

void ReadArgs::Usage()
{
    fprintf(stderr, "Usage:\n"
    		        "%s [-i <infile>] [-o <outfile>] [-t] [-s] [-g] [-q <n>] \n"
    		        "\n"
    		        "-i <infile>  Read input program (C/C++) from <infile>.\n"
    		        "-o <outfile> Write output program to <outfile>. C/C++ by default. Writes to stdout if omitted.\n"
    		        "-t           Turn on tracing internals (very verbose).\n"
    		        "-s           Run self-tests.\n"
	                "-gi          Generate Graphviz graphs for output or intermediate if used with -q.\n"
	                "-gp          Generate Graphviz graphs for search/replace patterns.\n"
	                "-q <n>       Stop after <n> steps. <n> may be 0 to exercise just parser and renderer.\n"
                    "\n"
    		        "One of -i or -s required; all others are optional.\n",
    		        exename.c_str() );
    exit(1);
}

void ReadArgs::Read( int argc, char *argv[] )
{ 
    int i=1;
	exename = argv[0];
    trace = false;
    intermediate_graph = false;
    pattern_graph = false;
    selftest = false;
    quitafter = 0x7fffffff;
    quitenable = false;
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
        else if( argv[i]==std::string("-gi") )
        {
        	intermediate_graph = true;
        }
        else if( argv[i]==std::string("-gp") )
        {
        	pattern_graph = true;
        }
        else if( argv[i]==std::string("-s") )
        {
            selftest = true;
        }
        else if( argv[i]==std::string("-q") )
        {
        	quitafter = strtoul( argv[++i], NULL, 10 );
        	quitenable = true;
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
