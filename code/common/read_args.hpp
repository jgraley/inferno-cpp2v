#ifndef READ_ARGS_HPP
#define READ_ARGS_HPP

#include <string>

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
    static bool graph;
    static void Usage();
    static void Read( int argc, char *argv[] );
    static bool trace;
    static int quitafter;
    static bool selftest;
};

#endif
