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

struct RenderKit 
{	
	RendererInterface *renderer;
};


class Render : public RendererInterface
{
public:	
    Render( string output_x_path_ = string() );
    Render( Syntax::Policy default_policy_, string output_x_path_ = string() );
    string RenderToString( shared_ptr<VN::CompareReplace> pattern );
    void WriteToFile(string s);
    
	static Syntax::Policy GetDefaultPolicy();
	string RenderIntoProduction( TreePtr<Node> node, 
	                             Syntax::Production surround_prod ) final;
	string RenderIntoProduction( TreePtr<Node> node, 
	                             Syntax::Production surround_prod, 
	                             Syntax::Policy policy ) final;
	string RenderMaybeInitAssignment( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy );
	string RenderMaybeBoot( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy );
	string RenderMaybeSemicolon( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy );
	string MaybeRenderPreRestriction( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy );
	string RenderNullPointer( Syntax::Production surround_prod );

	virtual string Dispatch( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy );

	string RenderNodeExplicit( shared_ptr<const Node> node, Syntax::Production surround_prod, Syntax::Policy policy );
	string RenderScopeResolvingPrefix( TreePtr<Node> id ) override;
	string GetUniqueIdentifierName( TreePtr<Node> id ) const override;

	virtual Syntax::Production GetNodeProduction( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy ) const;						 
	TreePtr<CPPTree::Scope> TryGetScope( TreePtr<Node> node ) const;
	bool IsDeclared( TreePtr<CPPTree::Identifier> id );
							 
	string RenderMismatchException( string fname, const Mismatch &me );

    const Syntax::Policy default_policy;
    TreePtr<Node> context;
    stack< TreePtr<Node> > scope_stack;
    queue<TreePtr<CPPTree::Instance>> definitions;
    UniquifyNames::NodeToNameMap unique_coupling_names;
    UniquifyNames::LinkSetByNode incoming_links_map;
    const string output_x_path;                                     
    SimpleCompare sc;
    unique_ptr<DefaultTransUtils> utils;
    TransKit trans_kit;
    Indenter indenter;
};
};

#endif

