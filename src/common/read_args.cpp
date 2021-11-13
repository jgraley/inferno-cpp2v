
#include "read_args.hpp"
#include "trace.hpp"
#include <string.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std;

string ReadArgs::exename;
string ReadArgs::infile;
string ReadArgs::outfile;
bool ReadArgs::intermediate_graph = false;
int ReadArgs::pattern_graph_index = -1; // -1 disables
string ReadArgs::pattern_graph_name = ""; // "" disables
bool ReadArgs::graph_trace = false;
bool ReadArgs::graph_dark = false;
bool ReadArgs::trace = false;
bool ReadArgs::trace_hits = false;
bool ReadArgs::trace_quiet = false;
bool ReadArgs::trace_no_stack = false;
string ReadArgs::hits_format;
bool ReadArgs::selftest = false;
int ReadArgs::runonlystep = 0; 
bool ReadArgs::runonlyenable = false; 
bool ReadArgs::quitafter = false;
Progress ReadArgs::quitafter_progress;
vector<int> ReadArgs::quitafter_counts;
int ReadArgs::repetitions = 100; // default behaviour
bool ReadArgs::rep_error = true; // default behaviour
bool ReadArgs::assert_pedigree = false;
bool ReadArgs::documentation_graphs = false;
bool ReadArgs::output_all = false;
bool ReadArgs::new_feature = false;

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
                    "-ts         Trace but don't show mini-stacks (for when re-architecting).\n"
    		        "-s          Run self-tests and quit.\n"
    		        "-ap         Enable pedigree assertions in search and replace engine.\n"
                    "-q<p>.<c>...   Stop after stage+step <p>, and optional match count(s) <c>. Eg -qT12.2.3\n"
                    "               for transformation 12, master match 2, first slave match 3\n"    
                    "               Note: -qT<n> makes -t and -r operate only on step n.\n"                
                    "               Note: if quitting after parse or later, output is attempted.\n"                
                    "-n<n>       Only run step <n>. User must ensure input program meets any restrictions of the step.\n"                    
	                "-g[t][k]i      Generate Graphviz dot file for output or intermediate if used with -q.\n"
	                "-g[t][k]p<n>   Generate dot file for specified transformation step n or by name,\n"
                    "               or generate all into a directory if name ends in /\n"
	                "-g[t][k]d      Generate dot files for documentation; -o specifies directory.\n"
	                "            Note: t enables trace details in graph; k enables dark colour-scheme.\n"
                    "-rn<n>      Stop search and replace after n repetitions and do not generate an error.\n"
                    "-re<n>      Stop search and replace after n repetitions and do generate an error.\n"
                    "-f          Output all intermediates: .cpp and .dot. <outfile> is path/basename.\n"
                    "-x          Enable new feature. Meaning varies.\n",
    		        exename.c_str() );
    exit(1);
}

string ReadArgs::GetArg( int al )
{
    if( strlen(argv[curarg]) > al+1 )
    {
        return string( argv[curarg]+al+1 );
    }
    else
    {
        curarg++;
        if(curarg >= argc)
            Usage();
        return string( argv[curarg] );
    }    
}

ReadArgs::ReadArgs( int ac, char *av[] )
{ 
    argc = ac;
    argv = av;
	exename = argv[0];
    for( curarg=1; curarg<argc; curarg++ )
    {
    	if( argv[curarg][0] != '-' || ((string)(argv[curarg])).size()<2 )
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
            char trace_option = argv[curarg][2];
            if( trace_option=='\0' )
            {
                trace = true;
            }
            else if( trace_option=='h' )
            {                
                trace_hits = true;
                hits_format = GetArg(2);
            }
            else if( trace_option=='q' )
            {
                trace_quiet = true;
            }
            else if( trace_option=='s' )
            {
                trace = true;
                trace_no_stack = true;
            }
            else
            {
                Usage();
            }
        }
        else if( option=='g' )
        {
            int ai = 2;
            char graph_option = argv[curarg][ai];
            if( graph_option=='t' )
            {
                graph_trace = true;
                ai++;
                graph_option = argv[curarg][ai];
            }
            if( graph_option=='k' )
            {
                graph_dark = true;
                ai++;
                graph_option = argv[curarg][ai];
            }
                
            if( graph_option=='i' )
            {
                intermediate_graph = true;
            }
            else if( graph_option=='p' )
            {
                string s = GetArg(ai);
                int v = strtoul( s.c_str(), nullptr, 10 );
                if( v==0 && s!="0" ) // Did strtoul fail?
                    pattern_graph_name = s;
                else
                    pattern_graph_index = v;
            }
            else if( graph_option=='d' )
            {
                documentation_graphs = true;
                string s = GetArg(ai);
                int v = strtoul( s.c_str(), nullptr, 10 );
                if( v==0 && s!="0" ) // Did strtoul fail?
                    pattern_graph_name = s;
                else
                    pattern_graph_index = v;
            }
            else
            {
                Usage();
            }
        }
        else if( option=='r' )
        {
            char reps_option = argv[curarg][2];
            if( reps_option=='e' )
                rep_error = true;
            else if( reps_option=='n' )
                rep_error = false;
            else
                Usage();
            repetitions = strtoul( GetArg(2).c_str(), nullptr, 10 );
        }
        else if( option=='s' )
        {
            selftest = true;
        }
        else if( option=='a' )
        {
            char assert_option = argv[curarg][2];
            if( assert_option=='p' )
                assert_pedigree = true;
            else
                Usage();
        }
        else if( option=='q' )
        {
            ParseQuitAfter( string(argv[curarg]+2) );
        }
        else if( option=='n' )
        {
        	runonlystep = strtoul( GetArg().c_str(), nullptr, 10 );
        	runonlyenable = true;
        }
        else if( option=='f' )
        {
            output_all = true;
            graph_trace = true;
        }
        else if( option=='x' )
        {
            new_feature = true;
        }
        else 
        {
            Usage();
        }
    }    
}

// quitafter syntax
// "<stage><step>.<sub0>.<sub1>.<sub2>.<master>.<slave1>.<slave2>.<slave3>"
// or "p" for parse
void ReadArgs::ParseQuitAfter(string arg)
{
    quitafter = true;

    int p = 0;
    int i = 0;
    int dot = 0;
    bool first = true;
    do
    {
        dot = arg.find('.', p);
        string s;
        if( dot != string::npos )   
            s = arg.substr(p, dot-p);
        else 
            s = arg.substr(p);
        if( first )
        {
            quitafter_progress = Progress( s );
            if( !quitafter_progress.IsValid() )
            {
                cerr << "Invalid stage/step string used with -q: " << s << endl;
                exit(1);
            }            
        }
        else
        {
            int v = atoi(s.c_str());
            quitafter_counts.push_back(v);            
        }
        p = dot+1;
        first = false;
    } while( dot != string::npos );
}
