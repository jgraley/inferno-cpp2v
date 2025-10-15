#ifndef RENDER_HPP
#define RENDER_HPP

#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "helpers/transformation.hpp"
#include "uniquify_identifiers.hpp"
#include "helpers/simple_compare.hpp"
#include "tree/misc.hpp"

namespace VN 
{
class CompareReplace; 

struct RenderKit : TransKit
{	
	function<string(string prefix, TreePtr<Node> node, Syntax::Production prod)> render;
};


class Render
{
public:	
    Render( string of = string() );
    string RenderToString( shared_ptr<VN::CompareReplace> pattern );
    string RenderToString( TreePtr<Node> root, string prefix="" );
    void WriteToFile(string s);
    
protected:
	string RenderIntoProduction( const RenderKit &kit, TreePtr<Node> node, Syntax::Production surround_prod );
	string RenderIntoProduction2( const RenderKit &kit, string prefix, TreePtr<Node> node, Syntax::Production surround_prod );
	string RenderNullPointer( const RenderKit &kit, string prefix, Syntax::Production surround_prod );
	virtual string Dispatch( const RenderKit &kit, string prefix, TreePtr<Node> node, Syntax::Production surround_prod );
	string RenderSpecial( const RenderKit &kit, string prefix, TreePtr<Node> node, Syntax::Production surround_prod );
	string RenderAny( const RenderKit &kit, string prefix, TreePtr<Node> node, unsigned enables = 0xFFFFFFFF );
						 
	TreePtr<CPPTree::Scope> TryGetScope( TreePtr<CPPTree::Identifier> id );
	bool IsDeclared( const RenderKit &kit, TreePtr<CPPTree::Identifier> id );
							 
	string RenderMismatchException( string fname, const Mismatch &me );

    TreePtr<Node> context;
    stack< TreePtr<Node> > scope_stack;
    queue<TreePtr<CPPTree::Instance>> definitions;
    UniquifyIdentifiers::IdentifierNameMap unique_ids;
    const string outfile;                                     
    SimpleCompare sc;
    
    RenderKit kit;
};
};

#endif

