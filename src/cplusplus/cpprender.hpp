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
	Syntax::Production GetNodeProduction( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy ) const final;
	
	string Dispatch( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy ) override;
	
    string RenderLiteral( TreePtr<CPPTree::Literal> sp, Syntax::Production surround_prod, Syntax::Policy policy ); 
    string RenderScopeResolvingPrefix( TreePtr<Node> node, Syntax::Policy policy ) final;
	string GetUniqueIdentifierName( TreePtr<Node> id ) const final;
    string DispatchTypeAndDeclarator( TreePtr<Node> type, string declarator, 
                                      Syntax::Production object_prod, Syntax::Production surround_prod, Syntax::Policy policy,
                                      bool constant ) final;
	string DispatchInternal( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy, Syntax::Refusal &ex );
    string RenderMacroStatement( TreePtr<CPPTree::MacroStatement> ms, Syntax::Production surround_prod, Syntax::Policy policy );
    string RenderStatementExpression( TreePtr<CPPTree::StatementExpression> ce, Syntax::Production surround_prod, Syntax::Policy policy );
    string RenderRecordInitialiser( TreePtr<CPPTree::RecordInitialiser> make_rec, Syntax::Production surround_prod, Syntax::Policy policy );

	string RenderMacroField( TreePtr<CPPTree::MacroField> md, Syntax::Production surround_prod, Syntax::Policy policy );
    string RenderTypedef( TreePtr<CPPTree::Typedef> t, Syntax::Production surround_prod, Syntax::Policy policy );
	string RenderEnum( TreePtr<CPPTree::Record> record, Syntax::Policy policy );

	UniquifyNames::NodeToNameMap unique_identifier_names;
	int nodes_not_rendered_to_c;
};

#endif
