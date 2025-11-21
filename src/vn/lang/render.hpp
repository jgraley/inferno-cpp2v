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
    string RenderToString( shared_ptr<VN::CompareReplace> pattern );
    void WriteToFile(string s);
    
	string RenderIntoProduction( TreePtr<Node> node, 
	                             Syntax::Production surround_prod, 
	                             Syntax::Policy policy = Syntax::Policy() ) final;
	string RenderConcreteIntoProduction( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy );
	string MaybeRenderPreRestriction( TreePtr<Node> node, Syntax::Production &surround_prod ) const;
	string RenderNullPointer( Syntax::Production surround_prod );

	virtual string Dispatch( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy );

	string RenderNodeExplicit( shared_ptr<const Node> node );
	string RenderScopeResolvingPrefix( TreePtr<Node> id ) override;
	string GetUniqueIdentifierName( TreePtr<Node> id ) const override;

	virtual Syntax::Production GetNodeProduction( TreePtr<Node> node, Syntax::Policy policy ) const;						 
	TreePtr<CPPTree::Scope> TryGetScope( TreePtr<Node> node ) const;
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
    TransKit trans_kit;
    Indenter indenter;
};
};

#endif

