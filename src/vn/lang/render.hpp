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
    
private:
    TreePtr<Node> context;
    stack< TreePtr<Node> > scope_stack;
    queue<TreePtr<CPPTree::Instance>> definitions;
    UniquifyIdentifiers::IdentifierNameMap unique_ids;
	string RenderIntoProduction( const Render::Kit &kit, TreePtr<Node> node, Syntax::Production prod );
	string Dispatch( const Render::Kit &kit, TreePtr<Node> node, Syntax::Production surround_prod );
	string RenderProgram( const Render::Kit &kit, TreePtr<CPPTree::Program> program, Syntax::Production surround_prod ); 
    string RenderLiteral( const Render::Kit &kit, TreePtr<CPPTree::Literal> sp, Syntax::Production surround_prod ); 
    string RenderPureIdentifier( const Render::Kit &kit, TreePtr<CPPTree::Identifier> id, Syntax::Production surround_prod );
    string ScopeResolvingPrefix( const Render::Kit &kit, TreePtr<CPPTree::Identifier> id, Syntax::Production surround_prod );
    string RenderIdentifier( const Render::Kit &kit, TreePtr<CPPTree::Identifier> id, Syntax::Production surround_prod );
    string RenderIntegral( const Render::Kit &kit, TreePtr<CPPTree::Integral> type, Syntax::Production surround_prod );
    string RenderIntegralTypeAndDeclarator( const Render::Kit &kit, TreePtr<CPPTree::Integral> type, string declarator );
    string RenderFloating( const Render::Kit &kit, TreePtr<CPPTree::Floating> type, Syntax::Production surround_prod );
    string RenderTypeAndDeclarator( const Render::Kit &kit, TreePtr<CPPTree::Type> type, string declarator, 
                                    Syntax::Production object_prod, Syntax::Production surround_prod, bool constant=false );
    string RenderType( const Render::Kit &kit, TreePtr<CPPTree::Type> type, Syntax::Production surround_prod );
    string Sanitise( string s );
    string RenderOperator( const Render::Kit &kit, TreePtr<CPPTree::Operator> op, Syntax::Production surround_prod );
    string RenderCallArgs( const Render::Kit &kit, TreePtr<CPPTree::Call> call );
    string RenderCall( const Render::Kit &kit, TreePtr<CPPTree::Call> call, Syntax::Production surround_prod );
    string RenderExprSeq( const Render::Kit &kit, Sequence<CPPTree::Expression> seq );
    string RenderExteriorCall( const Render::Kit &kit, TreePtr<CPPTree::SeqArgsCall> call, Syntax::Production surround_prod );
    string RenderMacroStatement( const Render::Kit &kit, TreePtr<CPPTree::MacroStatement> ms, Syntax::Production surround_prod );
    string RenderExpression( const Render::Kit &kit, TreePtr<CPPTree::Initialiser> expression, Syntax::Production surround_prod );
    string RenderMakeRecord( const Render::Kit &kit, TreePtr<CPPTree::RecordLiteral> make_rec, Syntax::Production surround_prod );
    string RenderMapInOrder( const Render::Kit &kit, 
                             TreePtr<CPPTree::IdValueMap> ro,
                             Sequence<CPPTree::Declaration> key_sequence );
	Sequence<CPPTree::Expression> SortMapOperands( TreePtr<CPPTree::IdValueMap> ro,
                                                   Sequence<CPPTree::Declaration> key_sequence );  
    string RenderAccessSpec( const Render::Kit &kit, TreePtr<CPPTree::AccessSpec> access, Syntax::Production surround_prod );
    string RenderStorage( const Render::Kit &kit, TreePtr<CPPTree::Instance> st ); // No actual storage node
    void ExtractInits( const Render::Kit &kit, Sequence<CPPTree::Statement> &body, 
                       Sequence<CPPTree::Statement> &inits, 
                       Sequence<CPPTree::Statement> &remainder );

    string RenderInstanceProto( const Render::Kit &kit, TreePtr<CPPTree::Instance> o );
	bool IsDeclared( const Render::Kit &kit, TreePtr<CPPTree::Identifier> id );
    string RenderInitialisation( const Render::Kit &kit, TreePtr<CPPTree::Initialiser> init );
    string RenderInstance( const Render::Kit &kit, TreePtr<CPPTree::Instance> o, Syntax::Production surround_prod );
    bool ShouldSplitInstance( const Render::Kit &kit, TreePtr<CPPTree::Instance> o );
	string RenderMacroDeclaration( const Render::Kit &kit, TreePtr<CPPTree::MacroDeclaration> md, Syntax::Production surround_prod );
    string RenderRecordProto( const Render::Kit &kit, TreePtr<CPPTree::Record> record );
	string RenderPreProcDecl(const Render::Kit &kit, TreePtr<CPPTree::PreProcDecl> ppd, Syntax::Production surround_prod );
    string RenderDeclaration( const Render::Kit &kit, TreePtr<CPPTree::Declaration> declaration, Syntax::Production surround_prod );
    string RenderStatement( const Render::Kit &kit, TreePtr<CPPTree::Statement> statement, Syntax::Production surround_prod );
 	string RenderConstructorInitList( const Render::Kit &kit, 
									  Sequence<CPPTree::Statement> spe );
	string RenderEnumBodyScope( const Render::Kit &kit, TreePtr<CPPTree::Record> record );
    string RenderOperandSequence( const Render::Kit &kit, Sequence<CPPTree::Expression> spe );
	string MaybeRenderFieldAccess( const Render::Kit &kit, TreePtr<CPPTree::Declaration> declaration,
			  			           TreePtr<CPPTree::AccessSpec> *current_access );
	string MaybeRenderAccessColon( const Render::Kit &kit, TreePtr<CPPTree::AccessSpec> this_access,
			  			           TreePtr<CPPTree::AccessSpec> *current_access );
    string RenderDeclScope( const Render::Kit &kit, 
							TreePtr<CPPTree::DeclScope> decl_scope,
							TreePtr<CPPTree::AccessSpec> init_access = TreePtr<CPPTree::AccessSpec>() );

	string RenderParams( const Render::Kit &kit, 
						 TreePtr<CPPTree::CallableParams> key);
						 
	TreePtr<CPPTree::Scope> TryGetScope( TreePtr<CPPTree::Identifier> id );
							 
	string RenderMismatchException( string fname, const Mismatch &me );
    const string outfile;                                     
    SimpleCompare sc;
};
};

#endif

