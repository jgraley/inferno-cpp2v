#ifndef RENDER_HPP
#define RENDER_HPP

#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "helpers/transformation.hpp"
#include "uniquify_identifiers.hpp"
#include "helpers/simple_compare.hpp"

namespace VN 
{
    class CompareReplace; // TODO all of Render should be VN::
};

class Render
{
public:
    Render( string of = string() );
    string RenderToString( shared_ptr<VN::CompareReplace> pattern );
    string RenderToString( TreePtr<Node> root );
    void WriteToFile(string s);
    
private:
    TreePtr<Node> temp_old_root;
    TreePtr<CPPTree::Scope> root_scope;
    string deferred_decls;
    stack< TreePtr<Node> > scope_stack;
    UniquifyIdentifiers unique;
    // Remember the orders of collections when we sort them. Mirrors the same
    // map in the parser.
    map< TreePtr<Node>, Sequence<CPPTree::Declaration> > backing_ordering;

    bool IsSystemC( const TransKit &kit, TreePtr<Node> root );
    string RenderLiteral( const TransKit &kit, TreePtr<CPPTree::Literal> sp );
    string RenderIdentifier( const TransKit &kit, TreePtr<CPPTree::Identifier> id );
    string RenderScopePrefix( const TransKit &kit, TreePtr<CPPTree::Identifier> id );
    string RenderScopedIdentifier( const TransKit &kit, TreePtr<CPPTree::Identifier> id, bool bracketize_cpp_scope=false );
    string RenderIntegralType( const TransKit &kit, TreePtr<CPPTree::Integral> type, string object=string() );
    string RenderFloatingType( const TransKit &kit, TreePtr<CPPTree::Floating> type );
    string RenderType( const TransKit &kit, TreePtr<CPPTree::Type> type, string object=string(), bool constant=false );
    string Sanitise( string s );
    string RenderOperator( const TransKit &kit, TreePtr<CPPTree::Operator> op, Sequence<CPPTree::Expression> &operands );
    string RenderCall( const TransKit &kit, TreePtr<CPPTree::Call> call );
    string RenderExpression( const TransKit &kit, TreePtr<CPPTree::Initialiser> expression, bool bracketize=false, bool bracketize_cpp_scope=false );
    string RenderMakeRecord( const TransKit &kit, TreePtr<CPPTree::MakeRecord> ro );
    string RenderMapInOrder( const TransKit &kit, 
                             TreePtr<CPPTree::MapOperator> ro,
                             TreePtr<Node> key );
    string RenderAccess( const TransKit &kit, TreePtr<CPPTree::AccessSpec> current_access );
    string RenderStorage( const TransKit &kit, TreePtr<CPPTree::Instance> st );
    void ExtractInits( const TransKit &kit, Sequence<CPPTree::Statement> &body, Sequence<CPPTree::Statement> &inits, Sequence<CPPTree::Statement> &remainder );

    string RenderInstanceProto( const TransKit &kit, TreePtr<CPPTree::Instance> o, 
                                bool out_of_line );
    string RenderInstance( const TransKit &kit, TreePtr<CPPTree::Instance> o,  
                           bool out_of_line );
    bool ShouldSplitInstance( const TransKit &kit, TreePtr<CPPTree::Instance> o );
    //string RenderRecordProto( const TransKit &kit, TreePtr<CPPTree::Record> record )
    string RenderDeclaration( const TransKit &kit, TreePtr<CPPTree::Declaration> declaration, 
                              bool force_incomplete = false );
    string RenderStatement( const TransKit &kit, TreePtr<CPPTree::Statement> statement );
 	string RenderConstructorInitList( const TransKit &kit, 
									  Sequence<CPPTree::Statement> spe );
	string RenderEnumBody( const TransKit &kit, 
                           Collection<CPPTree::Declaration> spe);
    string RenderOperandSequence( const TransKit &kit, 
                                  Sequence<CPPTree::Expression> spe );
    string RenderModuleCtor( const TransKit &kit, 
                             TreePtr<SCTree::Module> m,
                             TreePtr<CPPTree::AccessSpec> *access );    
    string RenderScope( const TransKit &kit, 
						TreePtr<CPPTree::Scope> key,
						TreePtr<CPPTree::AccessSpec> init_access = TreePtr<CPPTree::AccessSpec>() );
	string RenderParams( const TransKit &kit, 
						 TreePtr<CPPTree::CallableParams> key);
	string RenderMismatchException( string fname, const Mismatch &me );
    const string outfile;                                     
    SimpleCompare sc;
};

#endif

