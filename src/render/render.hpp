#ifndef RENDER_HPP
#define RENDER_HPP

#include "tree/tree.hpp"
#include "node/transformation.hpp"
#include "uniquify_identifiers.hpp"


class Render : public OutOfPlaceTransformation
{
public:
	using Transformation::operator();
    Render();
    TreePtr<Node> operator()( TreePtr<Node> context, TreePtr<Node> root );
private:
    TreePtr<Program> program;
    string deferred_decls;
    stack< TreePtr<Scope> > scope_stack;
    UniquifyIdentifiers unique;
    // Remember the orders of collections when we sort them. Mirrors the same
    // map in the parser.
    Map< TreePtr<Scope>, Sequence<Declaration> > backing_ordering;

    string RenderLiteral( TreePtr<Literal> sp );
    string RenderIdentifier( TreePtr<Identifier> id );
    string RenderScopePrefix( TreePtr<Identifier> id );
    string RenderScopedIdentifier( TreePtr<Identifier> id );
    string RenderIntegralType( TreePtr<Integral> type, string object=string() );
    string RenderFloatingType( TreePtr<Floating> type );
    string RenderType( TreePtr<Type> type, string object=string() );
    string Sanitise( string s );
    string RenderOperator( TreePtr<Operator> op, Sequence<Expression> &operands );
    string RenderCall( TreePtr<Call> call );
    string RenderExpression( TreePtr<Initialiser> expression, bool bracketize_operator=false );
    string RenderMakeRecord( TreePtr<MakeRecord> ro );
    string RenderMapInOrder( TreePtr<MapOperator> ro,
    		                 TreePtr<Scope> r,
                             string separator,
                             bool separate_last );
    string RenderAccess( TreePtr<AccessSpec> current_access );
    string RenderStorage( TreePtr<Instance> st );
    void ExtractInits( Sequence<Statement> &body, Sequence<Statement> &inits, Sequence<Statement> &remainder );
    string RenderInstance( TreePtr<Instance> o, string sep, bool showtype = true,
                           bool showstorage = true, bool showinit = true, bool showscope = false );
    bool ShouldSplitInstance( TreePtr<Instance> o );
    string RenderDeclaration( TreePtr<Declaration> declaration, string sep, TreePtr<AccessSpec> *current_access = NULL,
                              bool showtype = true, bool force_incomplete = false );
    string RenderStatement( TreePtr<Statement> statement, string sep );
    template< class ELEMENT >
    string RenderSequence( Sequence<ELEMENT> spe, 
                           string separator, 
                           bool separate_last,
                           TreePtr<AccessSpec> init_access = TreePtr<AccessSpec>(),
                           bool showtype=true );
    string RenderOperandSequence( Sequence<Expression> spe, 
                                  string separator, 
                                  bool separate_last );
    string RenderDeclarationCollection( TreePtr<Scope> sd,
			                            string separator, 
			                            bool separate_last,
			                            TreePtr<AccessSpec> init_access = TreePtr<AccessSpec>(),
			                            bool showtype=true );
};

#endif
