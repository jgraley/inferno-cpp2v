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

    bool IsSystemC( const TreeKit &kit, TreePtr<Node> root );
    string RenderLiteral( const TreeKit &kit, TreePtr<CPPTree::Literal> sp );
    string RenderIdentifier( const TreeKit &kit, TreePtr<CPPTree::Identifier> id );
    string RenderScopePrefix( const TreeKit &kit, TreePtr<CPPTree::Identifier> id );
    string RenderScopedIdentifier( const TreeKit &kit, TreePtr<CPPTree::Identifier> id );
    string RenderIntegralType( const TreeKit &kit, TreePtr<CPPTree::Integral> type, string object=string() );
    string RenderFloatingType( const TreeKit &kit, TreePtr<CPPTree::Floating> type );
    string RenderType( const TreeKit &kit, TreePtr<CPPTree::Type> type, string object=string(), bool constant=false );
    string Sanitise( string s );
    string RenderOperator( const TreeKit &kit, TreePtr<CPPTree::Operator> op, Sequence<CPPTree::Expression> &operands );
    string RenderCall( const TreeKit &kit, TreePtr<CPPTree::Call> call );
    string RenderExpression( const TreeKit &kit, TreePtr<CPPTree::Initialiser> expression, bool bracketize_operator=false );
    string RenderMakeRecord( const TreeKit &kit, TreePtr<CPPTree::MakeRecord> ro );
    string RenderMapInOrder( const TreeKit &kit, 
                             TreePtr<CPPTree::MapOperator> ro,
    		                 TreePtr<CPPTree::Scope> r,
                             string separator,
                             bool separate_last );
    string RenderAccess( const TreeKit &kit, TreePtr<CPPTree::AccessSpec> current_access );
    string RenderStorage( const TreeKit &kit, TreePtr<CPPTree::Instance> st );
    void ExtractInits( const TreeKit &kit, Sequence<CPPTree::Statement> &body, Sequence<CPPTree::Statement> &inits, Sequence<CPPTree::Statement> &remainder );
    string RenderInstance( const TreeKit &kit, TreePtr<CPPTree::Instance> o, string sep, bool showtype = true,
                           bool showstorage = true, bool showinit = true, bool showscope = false );
    bool ShouldSplitInstance( const TreeKit &kit, TreePtr<CPPTree::Instance> o );
    string RenderDeclaration( const TreeKit &kit, 
                              TreePtr<CPPTree::Declaration> declaration, 
                              string sep, 
                              TreePtr<CPPTree::AccessSpec> *current_access = nullptr,
                              bool showtype = true, 
                              bool force_incomplete = false,
                              bool shownonfuncinit = true );
    string RenderStatement( const TreeKit &kit, TreePtr<CPPTree::Statement> statement, string sep );
    template< class ELEMENT >
    string RenderSequence( const TreeKit &kit, 
                           Sequence<ELEMENT> spe, 
                           string separator, 
                           bool separate_last,
                           TreePtr<CPPTree::AccessSpec> init_access = TreePtr<CPPTree::AccessSpec>(),
                           bool showtype = true,
                           bool shownonfuncinit = true );
    string RenderOperandSequence( const TreeKit &kit, 
                                  Sequence<CPPTree::Expression> spe, 
                                  string separator, 
                                  bool separate_last );
    string RenderModuleCtor( const TreeKit &kit, 
                             TreePtr<SCTree::Module> m,
                             TreePtr<CPPTree::AccessSpec> *access );    
    string RenderDeclarationCollection( const TreeKit &kit, 
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

