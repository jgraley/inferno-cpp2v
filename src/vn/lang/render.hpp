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
	function<string(shared_ptr<const Node>, Syntax::Production surround_prod)> render_node;
};


class Render
{
public:	
    Render( string output_x_path_ = string() );
    string RenderToString( shared_ptr<VN::CompareReplace> pattern );
    void WriteToFile(string s);
    
	string RenderIntoProduction( TreePtr<Node> node, Syntax::Production surround_prod );
	string RenderConcreteIntoProduction( TreePtr<Node> node, Syntax::Production surround_prod );
	string MaybeRenderPreRestriction( TreePtr<Node> node,Syntax::Production &surround_prod ) const;
	string RenderNullPointer( Syntax::Production surround_prod );

	virtual string Dispatch( TreePtr<Node> node, Syntax::Production surround_prod );

	string RenderNodeOnly( shared_ptr<const Node> node, Syntax::Production surround_prod );
	string RenderNodeExplicit( shared_ptr<const Node> node );
	virtual Syntax::Production GetNodeProduction( TreePtr<Node> node ) const;
						 
	TreePtr<CPPTree::Scope> TryGetScope( TreePtr<CPPTree::Identifier> id );
	bool IsDeclared( TreePtr<CPPTree::Identifier> id );
							 
	string RenderMismatchException( string fname, const Mismatch &me );

    TreePtr<Node> context;
    stack< TreePtr<Node> > scope_stack;
    queue<TreePtr<CPPTree::Instance>> definitions;
    UniquifyNames::NodeToNameMap unique_coupling_names;
    UniquifyNames::LinkSetByNode incoming_links_map;
    const string output_x_path;                                     
    SimpleCompare sc;
    unique_ptr<DefaultTransUtils> utils;
    RenderKit kit;
    Indenter indenter;
};
};

#endif

