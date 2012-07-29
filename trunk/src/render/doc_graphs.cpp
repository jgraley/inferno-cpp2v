#include "tree/cpptree.hpp"
#include "helpers/transformation.hpp"
#include "sr/search_replace.hpp"
#include "sr/soft_patterns.hpp"
#include "common/trace.hpp"
#include "common/read_args.hpp"
#include "doc_graphs.hpp"
#include "graph.hpp"
#include "steps/inferno_patterns.hpp"
#include <inttypes.h>

using namespace CPPTree;

/// Output a graph of the subtree at node, to a dot file with basename
/// name and path supplied by user with -o (or local dir)
static void Output( TreePtr<Node> root, string name )
{
    string path = ReadArgs::outfile + name + string(".dot");
    TRACE("Generating graph ")(path)("\n");    
    Graph g( path );
    g( root ); 
}

/// Output a graph for search and replace documentation. For eg section 2.3a use major=2
/// minor=3 and subsection="a". If no letter, leave subsection empty.
static void OutputSR( TreePtr<Node> root, int major, int minor, string subsection, string tag )
{
    string name = SSPrintf( "srdoc_%d_%d%s_%s\n", major, minor, subsection.c_str(), tag.c_str() );
    Output(root, name); 
}

void GenerateDocumentationGraphs()
{
    MakeTreePtr<If> i;
    Output(i, "if"); 
}
