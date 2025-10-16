#ifndef CPPRENDER_HPP
#define CPPRENDER_HPP

#include "vn/lang/render.hpp"

class CppRender : public VN::Render
{
public:	
    CppRender( string of = string() );
	
private:	
	string Dispatch( string prefix, TreePtr<Node> node, Syntax::Production surround_prod ) override;
	string RenderProgram( const VN::RenderKit &kit, TreePtr<CPPTree::Program> program, Syntax::Production surround_prod ); 
	string RenderIdValuePair( const VN::RenderKit &kit, TreePtr<CPPTree::IdValuePair> ivp, Syntax::Production surround_prod );
    string RenderLiteral( const VN::RenderKit &kit, TreePtr<CPPTree::Literal> sp, Syntax::Production surround_prod ); 
    string RenderPureIdentifier( const VN::RenderKit &kit, TreePtr<CPPTree::Identifier> id, Syntax::Production surround_prod );
    string ScopeResolvingPrefix( const VN::RenderKit &kit, TreePtr<CPPTree::Identifier> id, Syntax::Production surround_prod );
    string RenderIdentifier( const VN::RenderKit &kit, TreePtr<CPPTree::Identifier> id, Syntax::Production surround_prod );
    string RenderIntegral( const VN::RenderKit &kit, TreePtr<CPPTree::Integral> type, Syntax::Production surround_prod );
    string RenderIntegralTypeAndDeclarator( const VN::RenderKit &kit, TreePtr<CPPTree::Integral> type, string declarator );
    string RenderFloating( const VN::RenderKit &kit, TreePtr<CPPTree::Floating> type, Syntax::Production surround_prod );
    string RenderTypeAndDeclarator( const VN::RenderKit &kit, TreePtr<CPPTree::Type> type, string declarator, 
                                    Syntax::Production object_prod, Syntax::Production surround_prod, bool constant=false );
    string RenderType( const VN::RenderKit &kit, TreePtr<CPPTree::Type> type, Syntax::Production surround_prod );
    string Sanitise( string s );
    string RenderOperator( const VN::RenderKit &kit, TreePtr<CPPTree::Operator> op, Syntax::Production surround_prod );
    string RenderMapArgs( const VN::RenderKit &kit, TreePtr<CPPTree::Type> dest_type, Collection<CPPTree::IdValuePair> &args );
    string RenderCall( const VN::RenderKit &kit, TreePtr<CPPTree::Call> call, Syntax::Production surround_prod );
    string RenderExprSeq( const VN::RenderKit &kit, Sequence<CPPTree::Expression> seq );
    string RenderExteriorCall( const VN::RenderKit &kit, TreePtr<CPPTree::SeqArgsCall> call, Syntax::Production surround_prod );
    string RenderMacroStatement( const VN::RenderKit &kit, TreePtr<CPPTree::MacroStatement> ms, Syntax::Production surround_prod );
    string RenderExpression( const VN::RenderKit &kit, TreePtr<CPPTree::Initialiser> expression, Syntax::Production surround_prod );
    string RenderMakeRecord( const VN::RenderKit &kit, TreePtr<CPPTree::RecordLiteral> make_rec, Syntax::Production surround_prod );
	Sequence<CPPTree::Expression> SortMapById( Collection<CPPTree::IdValuePair> &id_value_map,
                                               Sequence<CPPTree::Declaration> key_sequence );  
    string RenderAccessSpec( const VN::RenderKit &kit, TreePtr<CPPTree::AccessSpec> access, Syntax::Production surround_prod );
    string RenderStorage( const VN::RenderKit &kit, TreePtr<CPPTree::Instance> st ); // No actual storage node
    void ExtractInits( const VN::RenderKit &kit, Sequence<CPPTree::Statement> &body, 
                       Sequence<CPPTree::Statement> &inits, 
                       Sequence<CPPTree::Statement> &remainder );

    string RenderInstanceProto( const VN::RenderKit &kit, TreePtr<CPPTree::Instance> o );
    string RenderInitialisation( const VN::RenderKit &kit, TreePtr<CPPTree::Initialiser> init );
    string RenderInstance( const VN::RenderKit &kit, TreePtr<CPPTree::Instance> o, Syntax::Production surround_prod );
    bool ShouldSplitInstance( const VN::RenderKit &kit, TreePtr<CPPTree::Instance> o );
	string RenderMacroDeclaration( const VN::RenderKit &kit, TreePtr<CPPTree::MacroDeclaration> md, Syntax::Production surround_prod );
    string RenderRecordProto( const VN::RenderKit &kit, TreePtr<CPPTree::Record> record );
	string RenderPreProcDecl(const VN::RenderKit &kit, TreePtr<CPPTree::PreProcDecl> ppd, Syntax::Production surround_prod );
    string RenderDeclaration( const VN::RenderKit &kit, TreePtr<CPPTree::Declaration> declaration, Syntax::Production surround_prod );
    string RenderStatement( const VN::RenderKit &kit, TreePtr<CPPTree::Statement> statement, Syntax::Production surround_prod );
 	string RenderConstructorInitList( const VN::RenderKit &kit, 
									  Sequence<CPPTree::Statement> spe );
	string RenderEnumBodyScope( const VN::RenderKit &kit, TreePtr<CPPTree::Record> record );
    string RenderOperandSequence( const VN::RenderKit &kit, Sequence<CPPTree::Expression> spe );
	string MaybeRenderFieldAccess( const VN::RenderKit &kit, TreePtr<CPPTree::Declaration> declaration,
			  			           TreePtr<CPPTree::AccessSpec> *current_access );
	string MaybeRenderAccessColon( const VN::RenderKit &kit, TreePtr<CPPTree::AccessSpec> this_access,
			  			           TreePtr<CPPTree::AccessSpec> *current_access );
    string RenderDeclScope( const VN::RenderKit &kit, 
							TreePtr<CPPTree::DeclScope> decl_scope,
							TreePtr<CPPTree::AccessSpec> init_access = TreePtr<CPPTree::AccessSpec>() );

	string RenderParams( const VN::RenderKit &kit, 
						 TreePtr<CPPTree::CallableParams> key);
};

#endif
