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
    string RenderToString( shared_ptr<VN::CompareReplace> pattern, bool lowering_for_render );
    void WriteToFile(string s);
    
	static Syntax::Policy GetDefaultPolicy();
	string DoRender( const TreePtrInterface *tpi, 
	                 Syntax::Production surround_prod, 
	                 Syntax::Policy policy ) final;
	string DoRenderPreserve( TreePtr<Node> node, 
							 Syntax::Production surround_prod, 
							 Syntax::Policy policy ) final;
	string RenderNoDesignation( TreePtr<Node> node, 
								Syntax::Production surround_prod, 
								Syntax::Policy policy );
	string AccomodateInit( TreePtr<Node> node, Syntax::Production node_prod, Syntax::Production surround_prod, Syntax::Policy policy );
	string AccomodateBoot( TreePtr<Node> node, Syntax::Production node_prod, Syntax::Production surround_prod, Syntax::Policy policy );
	string AccomodateSemicolon( TreePtr<Node> node, Syntax::Production node_prod, Syntax::Production surround_prod, Syntax::Policy policy );
	string AccomodatePreRestriction( TreePtr<Node> node, Syntax::Production node_prod, Syntax::Production surround_prod, Syntax::Policy policy );
	string RenderNullPointer(Syntax::Production surround_prod, Syntax::Policy policy);

	virtual string Dispatch( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy );

	list<string> PopulateItemStrings( shared_ptr<const Node> node, Syntax::Policy policy );
	static string RenderNodeTypeName( shared_ptr<const Node> node ); 
	string RenderNodeExplicit( shared_ptr<const Node> node, Syntax::Production surround_prod, Syntax::Policy policy ) final;
	string RenderScopeResolvingPrefix( TreePtr<Node> id, Syntax::Policy policy ) override;
	string GetUniqueIdentifierName( TreePtr<Node> id ) const override;

    string DoRenderTypeAndDeclarator( const TreePtrInterface *tpi, string declarator, 
                                      Syntax::Production declarator_prod, Syntax::Production surround_prod, Syntax::Policy policy,
                                      TreePtr<Node> constant ) final;
    string DoRenderTypeAndDeclaratorPreserve( TreePtr<Node> type, string declarator, 
                                              Syntax::Production declarator_prod, Syntax::Production surround_prod, Syntax::Policy policy,
                                              TreePtr<Node> constant ) final;
    string AccomodateBootTypeAndDeclarator( TreePtr<Node> type, string declarator, 
                                            Syntax::Production declarator_prod, Syntax::Production surround_prod, Syntax::Policy policy,
                                            TreePtr<Node> constant );
	virtual string DispatchTypeAndDeclarator( TreePtr<Node> type, string declarator, 
                                              Syntax::Production declarator_prod, Syntax::Production surround_prod, Syntax::Policy policy,
                                              TreePtr<CPPTree::Constancy> constant );	

	virtual Syntax::Production GetNodeProduction( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy ) const;						 
	TreePtr<Node> TryGetScope( TreePtr<Node> node ) const override;
	bool IsDeclared( TreePtr<CPPTree::Identifier> id );
							 
	string RenderMismatchException( string fname, const Mismatch &me );
	const TransKit *GetTransKit() const override;

    const Syntax::Policy default_policy;
    TreePtr<Node> context;
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

