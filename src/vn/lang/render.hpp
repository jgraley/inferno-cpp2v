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

class Render
{
public:
	struct Kit : TransKit
	{	
	};
	
    Render( string of = string() );
    string RenderToString( shared_ptr<VN::CompareReplace> pattern );
    string RenderToString( TreePtr<Node> root );
    void WriteToFile(string s);
    
protected:
	string RenderIntoProduction( const Render::Kit &kit, TreePtr<Node> node, Syntax::Production prod );
	string RenderNullPointer( const Render::Kit &kit, Syntax::Production surround_prod );
	virtual string Dispatch( const Render::Kit &kit, TreePtr<Node> node, Syntax::Production surround_prod );
	string RenderAny( const Render::Kit &kit, TreePtr<Node> node, unsigned enables = 0xFFFFFFFF );
						 
	TreePtr<CPPTree::Scope> TryGetScope( TreePtr<CPPTree::Identifier> id );
	bool IsDeclared( const Render::Kit &kit, TreePtr<CPPTree::Identifier> id );
							 
	string RenderMismatchException( string fname, const Mismatch &me );

    TreePtr<Node> context;
    stack< TreePtr<Node> > scope_stack;
    queue<TreePtr<CPPTree::Instance>> definitions;
    UniquifyIdentifiers::IdentifierNameMap unique_ids;
    const string outfile;                                     
    SimpleCompare sc;
};
};

#endif

