#ifndef RENDER_HPP
#define RENDER_HPP

#include "tree/cpptree.hpp"
#include "helpers/transformation.hpp"
#include "uniquify_identifiers.hpp"
#include "helpers/simple_compare.hpp"


class Render
{
public:
    Render( string of = string() );
    TreePtr<Node> GenerateRender( TreePtr<Node> context, TreePtr<Node> root );
    
private:
    TreePtr<CPPTree::Program> program, temp_old_program;
    string deferred_decls;
    stack< TreePtr<CPPTree::Scope> > scope_stack;
    UniquifyIdentifiers unique;
    // Remember the orders of collections when we sort them. Mirrors the same
    // map in the parser.
    map< TreePtr<CPPTree::Scope>, Sequence<CPPTree::Declaration> > backing_ordering;

    bool IsSystemC( const TransKit &kit, TreePtr<Node> root );
    string RenderLiteral( const TransKit &kit, TreePtr<CPPTree::Literal> sp );
    string RenderIdentifier( const TransKit &kit, TreePtr<CPPTree::Identifier> id );
    string RenderScopePrefix( const TransKit &kit, TreePtr<CPPTree::Identifier> id );
    string RenderScopedIdentifier( const TransKit &kit, TreePtr<CPPTree::Identifier> id );
    string RenderIntegralType( const TransKit &kit, TreePtr<CPPTree::Integral> type, string object=string() );
    string RenderFloatingType( const TransKit &kit, TreePtr<CPPTree::Floating> type );
    string RenderType( const TransKit &kit, TreePtr<CPPTree::Type> type, string object=string(), bool constant=false );
    string Sanitise( string s );
    string RenderOperator( const TransKit &kit, TreePtr<CPPTree::Operator> op, Sequence<CPPTree::Expression> &operands );
    string RenderCall( const TransKit &kit, TreePtr<CPPTree::Call> call );
    string RenderExpression( const TransKit &kit, TreePtr<CPPTree::Initialiser> expression, bool bracketize_operator=false );
    string RenderMakeRecord( const TransKit &kit, TreePtr<CPPTree::MakeRecord> ro );
    string RenderMapInOrder( const TransKit &kit, 
                             TreePtr<CPPTree::MapOperator> ro,
    		                 TreePtr<CPPTree::Scope> r,
                             string separator,
                             bool separate_last );
    string RenderAccess( const TransKit &kit, TreePtr<CPPTree::AccessSpec> current_access );
    string RenderStorage( const TransKit &kit, TreePtr<CPPTree::Instance> st );
    void ExtractInits( const TransKit &kit, Sequence<CPPTree::Statement> &body, Sequence<CPPTree::Statement> &inits, Sequence<CPPTree::Statement> &remainder );
    string RenderInstance( const TransKit &kit, TreePtr<CPPTree::Instance> o, string sep, bool showtype = true,
                           bool showstorage = true, bool showinit = true, bool showscope = false );
    bool ShouldSplitInstance( const TransKit &kit, TreePtr<CPPTree::Instance> o );
    string RenderDeclaration( const TransKit &kit, 
                              TreePtr<CPPTree::Declaration> declaration, 
                              string sep, 
                              TreePtr<CPPTree::AccessSpec> *current_access = nullptr,
                              bool showtype = true, 
                              bool force_incomplete = false,
                              bool shownonfuncinit = true );
    string RenderStatement( const TransKit &kit, TreePtr<CPPTree::Statement> statement, string sep );
    template< class ELEMENT >
    string RenderSequence( const TransKit &kit, 
                           Sequence<ELEMENT> spe, 
                           string separator, 
                           bool separate_last,
                           TreePtr<CPPTree::AccessSpec> init_access = TreePtr<CPPTree::AccessSpec>(),
                           bool showtype = true,
                           bool shownonfuncinit = true );
    string RenderOperandSequence( const TransKit &kit, 
                                  Sequence<CPPTree::Expression> spe, 
                                  string separator, 
                                  bool separate_last );
    string RenderModuleCtor( const TransKit &kit, 
                             TreePtr<SCTree::Module> m,
                             TreePtr<CPPTree::AccessSpec> *access );    
    string RenderDeclarationCollection( const TransKit &kit, 
                                        TreePtr<CPPTree::Scope> sd,
			                            string separator, 
			                            bool separate_last,
			                            TreePtr<CPPTree::AccessSpec> init_access = TreePtr<CPPTree::AccessSpec>(),
			                            bool showtype=true );
    string RenderMismatchException( string fname, const Mismatch &me );
    const string outfile;			                         
    SimpleCompare sc;
};

#endif

