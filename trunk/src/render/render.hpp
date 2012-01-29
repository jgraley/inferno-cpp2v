#ifndef RENDER_HPP
#define RENDER_HPP

#include "tree/cpptree.hpp"
#include "helpers/transformation.hpp"
#include "uniquify_identifiers.hpp"


class Render : public OutOfPlaceTransformation
{
public:
	using Transformation::operator();
    Render();
    TreePtr<Node> operator()( TreePtr<Node> context, TreePtr<Node> root );
private:
    TreePtr<CPPTree::Program> program;
    string deferred_decls;
    stack< TreePtr<CPPTree::Scope> > scope_stack;
    UniquifyIdentifiers unique;
    // Remember the orders of collections when we sort them. Mirrors the same
    // map in the parser.
    Map< TreePtr<CPPTree::Scope>, Sequence<CPPTree::Declaration> > backing_ordering;

    bool IsSystemC( TreePtr<Node> root );
    string RenderLiteral( TreePtr<CPPTree::Literal> sp );
    string RenderIdentifier( TreePtr<CPPTree::Identifier> id );
    string RenderScopePrefix( TreePtr<CPPTree::Identifier> id );
    string RenderScopedIdentifier( TreePtr<CPPTree::Identifier> id );
    string RenderIntegralType( TreePtr<CPPTree::Integral> type, string object=string() );
    string RenderFloatingType( TreePtr<CPPTree::Floating> type );
    string RenderType( TreePtr<CPPTree::Type> type, string object=string() );
    string Sanitise( string s );
    string RenderOperator( TreePtr<CPPTree::Operator> op, Sequence<CPPTree::Expression> &operands );
    string RenderCall( TreePtr<CPPTree::Call> call );
    string RenderExpression( TreePtr<CPPTree::Initialiser> expression, bool bracketize_operator=false );
    string RenderMakeRecord( TreePtr<CPPTree::MakeRecord> ro );
    string RenderMapInOrder( TreePtr<CPPTree::MapOperator> ro,
    		                 TreePtr<CPPTree::Scope> r,
                             string separator,
                             bool separate_last );
    string RenderAccess( TreePtr<CPPTree::AccessSpec> current_access );
    string RenderStorage( TreePtr<CPPTree::Instance> st );
    void ExtractInits( Sequence<CPPTree::Statement> &body, Sequence<CPPTree::Statement> &inits, Sequence<CPPTree::Statement> &remainder );
    string RenderInstance( TreePtr<CPPTree::Instance> o, string sep, bool showtype = true,
                           bool showstorage = true, bool showinit = true, bool showscope = false );
    bool ShouldSplitInstance( TreePtr<CPPTree::Instance> o );
    string RenderDeclaration( TreePtr<CPPTree::Declaration> declaration, string sep, TreePtr<CPPTree::AccessSpec> *current_access = NULL,
                              bool showtype = true, bool force_incomplete = false );
    string RenderStatement( TreePtr<CPPTree::Statement> statement, string sep );
    template< class ELEMENT >
    string RenderSequence( Sequence<ELEMENT> spe, 
                           string separator, 
                           bool separate_last,
                           TreePtr<CPPTree::AccessSpec> init_access = TreePtr<CPPTree::AccessSpec>(),
                           bool showtype=true );
    string RenderOperandSequence( Sequence<CPPTree::Expression> spe, 
                                  string separator, 
                                  bool separate_last );
    string RenderModuleCtor( TreePtr<SCTree::Module> m,
                             TreePtr<CPPTree::AccessSpec> *access );    
    string RenderDeclarationCollection( TreePtr<CPPTree::Scope> sd,
			                            string separator, 
			                            bool separate_last,
			                            TreePtr<CPPTree::AccessSpec> init_access = TreePtr<CPPTree::AccessSpec>(),
			                            bool showtype=true );
};

#endif

