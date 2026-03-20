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
	
	string DispatchInternal( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy );
    string RenderLiteral( TreePtr<CPPTree::Literal> sp, Syntax::Production surround_prod, Syntax::Policy policy ); 
    string RenderScopeResolvingPrefix( TreePtr<Node> node, Syntax::Policy policy ) final;
	string GetUniqueIdentifierName( TreePtr<Node> id ) const final;
    string RenderIdentifier( TreePtr<CPPTree::Identifier> id, Syntax::Production surround_prod );
    string DispatchTypeAndDeclarator( TreePtr<Node> type, string declarator, 
                                      Syntax::Production object_prod, Syntax::Production surround_prod, Syntax::Policy policy,
                                      bool constant ) final;
    string Sanitise( string s );
    string RenderOperator( TreePtr<CPPTree::Operator> op, Syntax::Production surround_prod, Syntax::Policy policy );
    string RenderCall( TreePtr<CPPTree::Call> call, Syntax::Production surround_prod, Syntax::Policy policy );
    string RenderMacroStatement( TreePtr<CPPTree::MacroStatement> ms, Syntax::Production surround_prod, Syntax::Policy policy );
    string RenderExpression( TreePtr<CPPTree::Initialiser> expression, Syntax::Production surround_prod, Syntax::Policy policy );
    string RenderRecordInitialiser( TreePtr<CPPTree::RecordInitialiser> make_rec, Syntax::Production surround_prod, Syntax::Policy policy );
    string RenderStorage( TreePtr<CPPTree::Instance> st, Syntax::Policy policy ); // No actual storage node
    void ExtractInits( Sequence<CPPTree::Statement> &body, 
                       Sequence<CPPTree::Statement> &inits, 
                       Sequence<CPPTree::Statement> &remainder );

    string RenderInstanceProto( TreePtr<CPPTree::Instance> o, Syntax::Production starting_declarator_prod, Syntax::Policy policy );
    string RenderInitialisation( TreePtr<CPPTree::Initialiser> init, Syntax::Policy policy );
    string RenderInstance( TreePtr<CPPTree::Instance> o, Syntax::Production surround_prod, Syntax::Policy policy );
    bool ShouldSplitInstance( TreePtr<CPPTree::Instance> o, Syntax::Production surround_prod, Syntax::Policy policy );
	string RenderMacroDeclaration( TreePtr<CPPTree::MacroDeclaration> md, Syntax::Production surround_prod, Syntax::Policy policy );
    string RenderEnumHead( TreePtr<CPPTree::Record> record, Syntax::Policy policy );
	string RenderPreProcDecl( TreePtr<CPPTree::PreProcDecl> ppd, Syntax::Production surround_prod, Syntax::Policy policy );
    string RenderDeclaration( TreePtr<CPPTree::Declaration> declaration, Syntax::Production surround_prod, Syntax::Policy policy );
 	string RenderConstructorInitList( Sequence<CPPTree::Statement> spe, Syntax::Policy policy );
	string RenderEnumBody( TreePtr<CPPTree::Record> record, Syntax::Policy policy );

	UniquifyNames::NodeToNameMap unique_identifier_names;
};

#endif
