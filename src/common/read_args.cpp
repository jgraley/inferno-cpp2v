
#include "read_args.hpp"
#include <string.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>

std::string ReadArgs::exename;
std::string ReadArgs::infile;
std::string ReadArgs::outfile;
bool ReadArgs::intermediate_graph = false;
int ReadArgs::pattern_graph = -1; // -1 disables
bool ReadArgs::trace = false;
bool ReadArgs::selftest = false;
int ReadArgs::quitafter = 0x7fffffff; // basically never
bool ReadArgs::quitenable = false;
int ReadArgs::repetitions = 100; // default behaviour
bool ReadArgs::rep_error = true; // default behaviour

void ReadArgs::Usage()
{
    fprintf(stderr, "Usage:\n"
    		        "%s [-i<infile>] [-o<outfile>] [-t] [-s] [-g<opts>] [-q<n>] \n"
    		        "\n"
    		        "-i<infile>  Read input program (C/C++) from <infile>.\n"
    		        "-o<outfile> Write output program to <outfile>. C/C++ by default. Writes to stdout if omitted.\n"
    		        "-t          Turn on tracing internals (very verbose).\n"
    		        "-s          Run self-tests.\n"
                    "-q<n>       Stop after <n> steps. <n> may be 0 to exercise just parser and renderer.\n"                    
	                "-gi         Generate Graphviz graphs for output or intermediate if used with -q.\n"
	                "-gp<n>      Generate Graphviz graphs for transformation step n.\n"
                    "-rn<n>      Stop search and replace after n repetitions and do not generate an error.\n"
                    "-re<n>      Stop search and replace after n repetitions and do generate an error.\n"
                    "\n"
    		        "One of -i, -s or -gp required; all others are optional.\n",
    		        exename.c_str() );
    exit(1);
}

std::string ReadArgs::GetArg( int al )
{
    if( strlen(argv[curarg]) > al+1 )
    {
        return std::string( argv[curarg]+al+1 );
    }
    else
    {
        curarg++;
        if(curarg >= argc)
            Usage();
        return std::string( argv[curarg] );
    }    
}

ReadArgs::ReadArgs( int ac, char *av[] )
{ 
    argc = ac;
    argv = av;
	exename = argv[0];
    for( curarg=1; curarg<argc; curarg++ )
    {
    	if( argv[curarg][0] != '-' || ((std::string)(argv[curarg])).size()<2 )
    		Usage();

        char option = argv[curarg][1];
        
        if( option=='i' )
        {
            infile = GetArg();
        }
        else if( option=='o' )
        {
            outfile = GetArg();
        }
        else if( option=='t' )
        {
            trace = true;
        }
        else if( option=='g' )
        {
            char option2 = argv[curarg][2];
            if( option2=='i' )
                intermediate_graph = true;
            else if( option2=='p' )
                pattern_graph = strtoul( GetArg(2).c_str(), NULL, 10 );
            else
                Usage();
        }
        else if( option=='r' )
        {
            char option2 = argv[curarg][2];
            if( option2=='e' )
                rep_error = true;
            else if( option2=='n' )
                rep_error = false;
            else
                Usage();
            repetitions = strtoul( GetArg(2).c_str(), NULL, 10 );
        }
        else if( option=='s' )
        {
            selftest = true;
        }
        else if( option=='q' )
        {
        	quitafter = strtoul( GetArg().c_str(), NULL, 10 );
        	quitenable = true;
        }
        else 
        {
            Usage();
        }
    }    
    
    // infile or selftest is always required
    if( infile.empty() && !selftest && pattern_graph==-1 )
        Usage();
}
