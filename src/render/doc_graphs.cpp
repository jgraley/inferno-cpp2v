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

static void Output( TreePtr<Node> root, string name )
{
    string path = ReadArgs::outfile + name + string(".dot");
    TRACE("Generating graph ")(path)("\n");    
    Graph g( path );
    g( root ); 
}

void GenerateDocumentationGraphs()
{
    MakeTreePtr<If> i;
    Output(i, "if"); 
}
