#ifndef CPPRENDER_HPP
#define CPPRENDER_HPP

#include "vn/lang/render.hpp"

#include <typeinfo>
#include <typeindex>

class CppRender : public VN::Render
{
public:	
    CppRender( string of = string() );
    string RenderToString( TreePtr<Node> root );
	
private:	
	static Syntax::Policy GetDefaultPolicy();
	Syntax::Production GetNodeProduction( TreePtr<Node> node, Syntax::Policy policy ) const final;
	
	string OnRefusal( Syntax::Refusal &ex, TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy ) override;
	
	string GetUniqueIdentifierName( TreePtr<Node> id ) const final;
    string DispatchTypeAndDeclarator( TreePtr<Node> type, string declarator, 
                                      Syntax::Production object_prod, Syntax::Production surround_prod, Syntax::Policy policy,
                                      TreePtr<CPPTree::Constancy> constant ) final;

	UniquifyNames::NodeToNameMap unique_identifier_names;
	int nodes_not_rendered_to_c;
};

#endif
