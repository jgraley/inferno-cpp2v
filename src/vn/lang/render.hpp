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

class RenderRecursionInterface
{
public:	
	virtual string RenderIntoProduction( TreePtr<Node> node, Syntax::Production surround_prod ) = 0;
	virtual string RenderNodeOnly( shared_ptr<const Node> node, Syntax::Production surround_prod ) = 0;
	virtual string ScopeResolvingPrefix( TreePtr<Node> id, Syntax::Production surround_prod ) = 0;
};


struct RenderKit : TransKit
{	
	RenderRecursionInterface *recurse;
	const UniquifyNames::NodeToNameMap *unique_identifier_names;
	const UniquifyNames::NodeToNameMap *unique_coupling_names;
};


class Render : public RenderRecursionInterface
{
public:	
    Render( string output_x_path_ = string() );
    string RenderToString( shared_ptr<VN::CompareReplace> pattern );
    void WriteToFile(string s);
    
	string RenderIntoProduction( TreePtr<Node> node, Syntax::Production surround_prod ) override;
	string RenderConcreteIntoProduction( TreePtr<Node> node, Syntax::Production surround_prod );
	string MaybeRenderPreRestriction( TreePtr<Node> node,Syntax::Production &surround_prod ) const;
	string RenderNullPointer( Syntax::Production surround_prod );

	virtual string Dispatch( TreePtr<Node> node, Syntax::Production surround_prod );

	string RenderNodeOnly( shared_ptr<const Node> node, Syntax::Production surround_prod ) override;
	string RenderNodeExplicit( shared_ptr<const Node> node );
	string ScopeResolvingPrefix( TreePtr<Node> id, Syntax::Production surround_prod ) override;
	
	virtual Syntax::Production GetNodeProduction( TreePtr<Node> node ) const;						 
	TreePtr<CPPTree::Scope> TryGetScope( TreePtr<Node> node );
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

