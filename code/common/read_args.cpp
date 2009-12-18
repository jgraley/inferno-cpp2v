
#include "read_args.hpp"
#include <string>
#include <stdlib.h>

std::string ReadArgs::exename;
std::string ReadArgs::infile;
std::string ReadArgs::outfile;
bool ReadArgs::intermediate_graph = false;
bool ReadArgs::pattern_graph = false;
bool ReadArgs::hack_graph = false;
bool ReadArgs::trace = false;
bool ReadArgs::selftest = false;
int ReadArgs::quitafter = 0x7fffffff; // basically never
bool ReadArgs::quitenable = false;

void ReadArgs::Usage()
{
    fprintf(stderr, "Usage:\n"
    		        "%s [-i<infile>] [-o<outfile>] [-t] [-s] [-g<opts>] [-q<n>] \n"
    		        "\n"
    		        "-i<infile>  Read input program (C/C++) from <infile>.\n"
    		        "-o<outfile> Write output program to <outfile>. C/C++ by default. Writes to stdout if omitted.\n"
    		        "-t          Turn on tracing internals (very verbose).\n"
    		        "-s          Run self-tests.\n"
	                "-gi         Generate Graphviz graphs for output or intermediate if used with -q.\n"
	                "-gp         Generate Graphviz graphs for search/replace patterns.\n"
	                "-q<n>       Stop after <n> steps. <n> may be 0 to exercise just parser and renderer.\n"
                    "\n"
    		        "One of -i or -s required; all others are optional.\n",
    		        exename.c_str() );
    exit(1);
}

void ReadArgs::Read( int argc, char *argv[] )
{ 
	exename = argv[0];
    for( int i=1; i<argc; i++ )
    {
    	if( argv[i][0] != '-' || ((std::string)(argv[i])).size()<2 )
    		Usage();

        if( argv[i][1]=='i' )
        {
            infile = argv[i]+2;
        }
        else if( argv[i][1]=='o' )
        {
            outfile = argv[i]+2;
        }
        else if( argv[i][1]=='t' )
        {
            trace = true;
        }
        else if( argv[i][1]=='g' )
        {
        	if( argv[i][2]=='i' )
        	    intermediate_graph = true;
        	else if( argv[i][2]=='p' )
        		pattern_graph = true;
        	else
        		Usage();

        	if( argv[i][3]=='h' )
        		hack_graph = true;
        }
        else if( argv[i][1]=='s' )
        {
            selftest = true;
        }
        else if( argv[i][1]=='q' )
        {
        	quitafter = strtoul( argv[i]+2, NULL, 10 );
        	quitenable = true;
        }
        else 
        {
            Usage();
        }
    }    
    
    // infile or selftest is always required
    if( infile.empty() && !selftest )
        Usage();
}
