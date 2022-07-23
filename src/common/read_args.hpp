#ifndef READ_ARGS_HPP
#define READ_ARGS_HPP

#include "progress.hpp"

#include <string>
#include <vector>

// Try to share one command line args parser between all executable
// targets so usage is consistent and to avoid duplciation. We allow
// globals here since in a way command line args *are* global. Avoid
// the word "parse" here. 

class ReadArgs
{
public:
    void Usage(string msg);
    std::string GetArg( int al=1 );
    ReadArgs( int argc, char *argv[] );

    static std::string exename;
    static std::string infile;
    static std::string outfile;
    static bool intermediate_graph;
    static int pattern_graph_index;
    static std::string pattern_graph_name;
    static bool graph_trace;
    static bool graph_dark;
    static bool trace;
    static bool trace_hits;    
    static bool trace_quiet;   
    static bool trace_no_stack; 
    static std::string hits_format;
    static bool quitafter;
    static Progress quitafter_progress;
    static vector<int> quitafter_counts;
    static int runonlystep;
    static bool runonlyenable;
    static int repetitions;
    static bool rep_error;
    static bool unit_tests;
    static bool csp_test;
    static bool assert_pedigree;
    static bool documentation_graphs;
    static bool output_all;
    static bool use_conv_out_loop;
    
private:
    void ParseQuitAfter(std::string arg);
    int curarg;
    char **argv;
    int argc;
};

#endif

