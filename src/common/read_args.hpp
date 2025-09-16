#ifndef READ_ARGS_HPP
#define READ_ARGS_HPP

#include "progress.hpp"

#include <string>
#include <vector>

// Try to share one command line args parser between all executable
// targets so usage is consistent and to avoid duplciation. We allow
// globals here since in a way command line args *are* global. Avoid
// the word "parse" here. 

using namespace std;


class ReadArgs
{
public:
    void Usage(string msg);
    string GetArg( size_t al=1 );
    ReadArgs( int argc, char *argv[] );

    static string exename;
    static string infile;
    static string outfile;
    static bool intermediate_graph;
    static int pattern_graph_index;
    static string pattern_graph_name;
    static int pattern_render_index;
    static string pattern_render_name;
    static bool graph_trace;
    static bool graph_dark;
    static bool trace;
    static bool trace_hits;    
    static bool trace_quiet;   
    static bool trace_no_stack; 
    static string hits_format;
    static bool quitafter;
    static Progress quitafter_progress;
    static vector<int> quitafter_counts;
    static int runonlystep;
    static bool runonlyenable;
    static int repetitions;
    static bool rep_error;
    static bool test_units;
    static bool test_csp;
    static bool test_db;
    static bool documentation_graphs;
    static bool output_all;
    
private:
    void ParseQuitAfter(string arg);
    int curarg;
    char **argv;
    int argc;
};

#endif

