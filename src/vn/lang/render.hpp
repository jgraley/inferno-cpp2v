#ifndef RENDER_HPP
#define RENDER_HPP

#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "helpers/transformation.hpp"
#include "uniquify_identifiers.hpp"
#include "helpers/simple_compare.hpp"
#include "tree/misc.hpp"
#include "indenter.hpp"

namespace VN 
{
class CompareReplace; 

struct RenderKit : TransKit
{	
	function<string(TreePtr<Node> node, Syntax::Production prod)> render;
};


class Render
{
public:	
    Render( string of = string() );
    string RenderToString( shared_ptr<VN::CompareReplace> pattern );
    void WriteToFile(string s);
    
	string RenderIntoProduction( TreePtr<Node> node, Syntax::Production surround_prod );
	string RenderConcreteIntoProduction( TreePtr<Node> node, Syntax::Production surround_prod );
	string RenderNullPointer( Syntax::Production surround_prod );
	virtual Syntax::Production GetNodeProduction( TreePtr<Node> node ) const;
	virtual string Dispatch( TreePtr<Node> node, Syntax::Production surround_prod );
						 
	TreePtr<CPPTree::Scope> TryGetScope( TreePtr<CPPTree::Identifier> id );
	bool IsDeclared( TreePtr<CPPTree::Identifier> id );
							 
	string RenderMismatchException( string fname, const Mismatch &me );

    TreePtr<Node> context;
    stack< TreePtr<Node> > scope_stack;
    queue<TreePtr<CPPTree::Instance>> definitions;
    UniquifyNames::NodeToNameMap unique_coupling_names;
    UniquifyNames::LinkSetByNode incoming_links;
    const string outfile;                                     
    SimpleCompare sc;
    unique_ptr<DefaultTransUtils> utils;
    RenderKit kit;
    Indenter indenter;
};
};

#endif

