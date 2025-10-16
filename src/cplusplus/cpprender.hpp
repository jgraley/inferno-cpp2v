#ifndef CPPRENDER_HPP
#define CPPRENDER_HPP

#include "vn/lang/render.hpp"

class CppRender : public VN::Render
{
public:	
    CppRender( string of = string() );
	
private:	
	string Dispatch( string prefix, TreePtr<Node> node, Syntax::Production surround_prod ) override;
	string RenderProgram( TreePtr<CPPTree::Program> program, Syntax::Production surround_prod ); 
	string RenderIdValuePair( TreePtr<CPPTree::IdValuePair> ivp, Syntax::Production surround_prod );
    string RenderLiteral( TreePtr<CPPTree::Literal> sp, Syntax::Production surround_prod ); 
    string RenderPureIdentifier( TreePtr<CPPTree::Identifier> id, Syntax::Production surround_prod );
    string ScopeResolvingPrefix( TreePtr<CPPTree::Identifier> id, Syntax::Production surround_prod );
    string RenderIdentifier( TreePtr<CPPTree::Identifier> id, Syntax::Production surround_prod );
    string RenderIntegral( TreePtr<CPPTree::Integral> type, Syntax::Production surround_prod );
    string RenderIntegralTypeAndDeclarator( TreePtr<CPPTree::Integral> type, string declarator );
    string RenderFloating( TreePtr<CPPTree::Floating> type, Syntax::Production surround_prod );
    string RenderTypeAndDeclarator( TreePtr<CPPTree::Type> type, string declarator, 
                                    Syntax::Production object_prod, Syntax::Production surround_prod, bool constant=false );
    string RenderType( TreePtr<CPPTree::Type> type, Syntax::Production surround_prod );
    string Sanitise( string s );
    string RenderOperator( TreePtr<CPPTree::Operator> op, Syntax::Production surround_prod );
    string RenderMapArgs( TreePtr<CPPTree::Type> dest_type, Collection<CPPTree::IdValuePair> &args );
    string RenderCall( TreePtr<CPPTree::Call> call, Syntax::Production surround_prod );
    string RenderExprSeq( Sequence<CPPTree::Expression> seq );
    string RenderExteriorCall( TreePtr<CPPTree::SeqArgsCall> call, Syntax::Production surround_prod );
    string RenderMacroStatement( TreePtr<CPPTree::MacroStatement> ms, Syntax::Production surround_prod );
    string RenderExpression( TreePtr<CPPTree::Initialiser> expression, Syntax::Production surround_prod );
    string RenderMakeRecord( TreePtr<CPPTree::RecordLiteral> make_rec, Syntax::Production surround_prod );
	Sequence<CPPTree::Expression> SortMapById( Collection<CPPTree::IdValuePair> &id_value_map,
                                               Sequence<CPPTree::Declaration> key_sequence );  
    string RenderAccessSpec( TreePtr<CPPTree::AccessSpec> access, Syntax::Production surround_prod );
    string RenderStorage( TreePtr<CPPTree::Instance> st ); // No actual storage node
    void ExtractInits( Sequence<CPPTree::Statement> &body, 
                       Sequence<CPPTree::Statement> &inits, 
                       Sequence<CPPTree::Statement> &remainder );

    string RenderInstanceProto( TreePtr<CPPTree::Instance> o );
    string RenderInitialisation( TreePtr<CPPTree::Initialiser> init );
    string RenderInstance( TreePtr<CPPTree::Instance> o, Syntax::Production surround_prod );
    bool ShouldSplitInstance( TreePtr<CPPTree::Instance> o );
	string RenderMacroDeclaration( TreePtr<CPPTree::MacroDeclaration> md, Syntax::Production surround_prod );
    string RenderRecordProto( TreePtr<CPPTree::Record> record );
	string RenderPreProcDecl( TreePtr<CPPTree::PreProcDecl> ppd, Syntax::Production surround_prod );
    string RenderDeclaration( TreePtr<CPPTree::Declaration> declaration, Syntax::Production surround_prod );
    string RenderStatement( TreePtr<CPPTree::Statement> statement, Syntax::Production surround_prod );
 	string RenderConstructorInitList( Sequence<CPPTree::Statement> spe );
	string RenderEnumBodyScope( TreePtr<CPPTree::Record> record );
    string RenderOperandSequence( Sequence<CPPTree::Expression> spe );
	string MaybeRenderFieldAccess( TreePtr<CPPTree::Declaration> declaration,
			  			           TreePtr<CPPTree::AccessSpec> *current_access );
	string MaybeRenderAccessColon( TreePtr<CPPTree::AccessSpec> this_access,
			  			           TreePtr<CPPTree::AccessSpec> *current_access );
    string RenderDeclScope( TreePtr<CPPTree::DeclScope> decl_scope,
							TreePtr<CPPTree::AccessSpec> init_access = TreePtr<CPPTree::AccessSpec>() );

	string RenderParams( TreePtr<CPPTree::CallableParams> key);
};

#endif
