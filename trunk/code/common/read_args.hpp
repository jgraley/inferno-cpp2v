#ifndef READ_ARGS_HPP
#define READ_ARGS_HPP

#include <string>

// Try to share one command line args parser between all executable
// targets so usage is consistent and to avoid duplciation. We allow
// globals here since in a way command line args are global. Avoid
// the word "parse" here. Maybe this lot should be statics in a class.

extern std::string infile;

void read_args( int argc, char *argv[] );

#endif
