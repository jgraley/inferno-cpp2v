#ifndef READ_ARGS_HPP
#define READ_ARGS_HPP

#include <string>
#include <vector>

// Try to share one command line args parser between all executable
// targets so usage is consistent and to avoid duplciation. We allow
// globals here since in a way command line args *are* global. Avoid
// the word "parse" here. 

class ReadArgs
{
public:
    static std::string exename;
    static std::string infile;
    static std::string outfile;
    static bool intermediate_graph;
    static int pattern_graph;
    void Usage();
    std::string GetArg( int al=1 );
    ReadArgs( int argc, char *argv[] );
    static bool trace;
    static bool trace_hits;    
    static bool trace_quiet;    
    static std::string hits_format;
    static std::vector<int> quitafter;
    static int runonlystep;
    static bool runonlyenable;
    static int repetitions;
    static bool rep_error;
    static bool selftest;
    static bool assert_pedigree;
    static bool documentation_graphs;
    static bool output_all;
private:
    void ParseQuitAfter(std::string arg);
    int curarg;
    char **argv;
    int argc;
};

#endif
