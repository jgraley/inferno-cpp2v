
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
bool ReadArgs::trace_hits = false;
bool ReadArgs::trace_quiet = false;
std::string ReadArgs::hits_format;
bool ReadArgs::selftest = false;
int ReadArgs::runonlystep = 0; 
bool ReadArgs::runonlyenable = false; 
int ReadArgs::quitafter = 0x7fffffff; // basically never
bool ReadArgs::quitenable = false;
int ReadArgs::repetitions = 100; // default behaviour
bool ReadArgs::rep_error = true; // default behaviour
bool ReadArgs::assert_pedigree = false;
bool ReadArgs::documentation_graphs = false;
bool ReadArgs::output_all = false;

void ReadArgs::Usage()
{
    fprintf(stderr, "Usage:\n"
    		        "%s <options> \n"
    		        "\n"
    		        "-i<infile>  Read input program (C/C++) from <infile>.\n"
    		        "-o<outfile> Write output program to <outfile>. C/C++ by default. Writes to stdout if omitted.\n"
    		        "-t          Turn on tracing internals (very verbose).\n"    		        
                    "-th<fmt>    Dump hit counts at the end of execution based on <fmt>.\n"
                    "            Note: use -th? for help on <fmt>.\n"
                    "-tq         No output to console.\n"
    		        "-s          Run self-tests.\n"
    		        "-ap         Enable pedigree assertions in search and replace engine.\n"
                    "-q<n>       Stop before step <n>. <n> may be 0 to exercise just parser and renderer.\n"    
                    "            Note: -q<n> makes -t and -r operate only on step n-1.\n"                
                    "-n<n>       Only run step <n>. User must ensure input program meets any restrictions of the step.\n"                    
	                "-gi         Generate Graphviz dot file for output or intermediate if used with -q.\n"
	                "-gp<n>      Generate dot file for specified transformation step n.\n"
	                "-gd         Generate dot files for documentation; -o specifies directory.\n"
                    "-rn<n>      Stop search and replace after n repetitions and do not generate an error.\n"
                    "-re<n>      Stop search and replace after n repetitions and do generate an error.\n"
                    "-f          Output all intermediates. <outfile> is path/basename.\n"
                    "\n"
    		        "One of -i, -s, -th, -gp, -gd required; all others are optional.\n",
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
            char option2 = argv[curarg][2];
            if( option2=='\0' )
                trace = true;
            else if( option2=='h' )
            {                
                trace_hits = true;
                hits_format = GetArg(2);
            }
            else if( option2=='q' )
                trace_quiet = true;
            else
                Usage();
        }
        else if( option=='g' )
        {
            char option2 = argv[curarg][2];
            if( option2=='i' )
                intermediate_graph = true;
            else if( option2=='p' )
                pattern_graph = strtoul( GetArg(2).c_str(), NULL, 10 );
            else if( option2=='d' )
                documentation_graphs = true;
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
        else if( option=='a' )
        {
            char option2 = argv[curarg][2];
            if( option2=='p' )
                assert_pedigree = true;
            else
                Usage();
        }
        else if( option=='q' )
        {
        	quitafter = strtoul( GetArg().c_str(), NULL, 10 );
        	quitenable = true;
        }
        else if( option=='n' )
        {
        	runonlystep = strtoul( GetArg().c_str(), NULL, 10 );
        	runonlyenable = true;
        }
        else if( option=='f' )
        {
            output_all = true;
        }
        else 
        {
            Usage();
        }
    }    
    
    // infile or selftest is always required
    if( infile.empty() && 
        !selftest && 
        pattern_graph==-1 && 
        !(trace_hits && hits_format==std::string("?") ) &&
        !documentation_graphs )
        Usage();
}