#include "tree/cpptree.hpp"
#include "helpers/transformation.hpp"
#include "tree/typeof.hpp"
#include "common/trace.hpp"
#include "common/read_args.hpp"
#include "tree/type_data.hpp"
#include "helpers/walk.hpp"
#include "helpers/simple_duplicate.hpp"
#include "tree/misc.hpp"
#include "tree/scope.hpp"
#include "sort_decls.hpp"
#include "render.hpp"
#include "clang/Parse/DeclSpec.h"
#include "uniquify_identifiers.hpp"
#include "search_replace.hpp"

using namespace CPPTree;
using namespace VN;

// Don't like the layout of rendered code?
// There's a .clang-format in repo root.

// TODO indent back to previous level at end of string
#define ERROR_UNKNOWN(V) \
    ( string( "\n«" ) + \
      string( V ) + \
      string( " not supported in " ) + \
      string( __func__ ) + \
      string( "()»\n" ) )

#define ERROR_UNSUPPORTED(P) \
    ERROR_UNKNOWN( P ? TYPE_ID_NAME(*P) : "<nullptr>" )

// For #400 make methods that return strings try-functions
// and use this for the catch clause.
#define DEFAULT_CATCH_CLAUSE \
    catch( const ::Mismatch &e ) \
    { \
        return RenderMismatchException( __func__, e ); \
    } 

Render::Render( string of ) :
    outfile( of )
{
}


string Render::RenderToString( shared_ptr<CompareReplace> pattern )
{
    return RenderToString( pattern->GetSearchComparePattern() ) +
           "\n꩜\n" +
           RenderToString( pattern->GetReplacePattern() );
}


string Render::RenderToString( TreePtr<Node> root )
{       
    // Context is used for various lookups but does not need
    // to be a Scope.
    context = root; 
        
    DefaultTransUtils utils(context);
    Render::Kit kit { &utils };

    // Make the identifiers unique (does its own tree walk)
    unique_ids = UniquifyIdentifiers::UniquifyAll( kit, context );

    return RenderIntoProduction( kit, root, Syntax::Production::PROGRAM );
}


void Render::WriteToFile( string s )
{
    if( outfile.empty() )
    {
        puts( s.c_str() );
    }
    else
    {
        FILE *fp = fopen( outfile.c_str(), "wt" );
        ASSERT( fp )( "Cannot open output file \"%s\"", outfile.c_str() );
        fputs( s.c_str(), fp );
        fclose( fp );
    }        
}


string Render::RenderIntoProduction( const Render::Kit &kit, TreePtr<Node> node, Syntax::Production surround_prod )
{
    INDENT("R");
    // Production surround_prod relates to the surrounding grammar production and can be 
    // used to change the render of a certain subtree. It represents all the ancestor nodes of
    // the one supplied.
    string s, ss;
    Syntax::Production node_prod = node->GetMyProduction();
    bool do_boot = Syntax::GetPrecedence(node_prod) < Syntax::GetPrecedence(surround_prod);    
    bool semicolon = Syntax::GetPrecedence(surround_prod) < Syntax::GetPrecedence(Syntax::Production::CONDITION) &&
                     Syntax::GetPrecedence(node_prod) > Syntax::GetPrecedence(Syntax::Production::PROTOTYPE);  
    bool do_init = surround_prod == Syntax::Production::INITIALISER;
    if( ReadArgs::use_feature_option=='c' )
		s += SSPrintf("\n// %s Surround %d node %d (%s) from %p boot: %s semcolon: %s init: $s\n", 
					 Tracer::GetPrefix().c_str(), 
					 Syntax::GetPrecedence(surround_prod), 
					 Syntax::GetPrecedence(node_prod),
					 Trace(node).c_str(), 
					 RETURN_ADDR(),
					 do_boot ? "yes" : "no",
					 semicolon ? "yes" : "no",
					 do_init ? "yes" : "no" );
        
    ASSERT( node_prod != Syntax::Production::UNDEFINED )
          ("Rendering expression: ")(node)(" surr prod: %d",(int)surround_prod)(" got no ideal production\n");
    
    switch(node_prod)
    {
        case Syntax::Production::BOOT_STATEMENT...Syntax::Production::TOP_STATEMENT: // Statement productions at different precedences
        {
            // If current production has too-high precedence, boot back down using braces
            if( do_boot )
            {
				// Braces can actually work in expressions, eg in {}. The nodes are STATEMENT_SEQ and we boot to BOOT_STATEMENT
				ASSERT( Syntax::GetPrecedence(surround_prod) <= Syntax::GetPrecedence(Syntax::Production::BRACED) ||			
				        Syntax::GetPrecedence(surround_prod) > Syntax::GetPrecedence(Syntax::Production::TOP_STATEMENT) )
					  ("Braces won't achieve high enough precedence for surrounding statement production\n")
                      ("Node: ")(node)("\n")
                      ("Surr prod: %d node prod: %d", (int)surround_prod, (int)node_prod); 
				ASSERT( Syntax::GetPrecedence(surround_prod) <= Syntax::GetPrecedence(Syntax::Production::PARENTHESISED) )
					  ("Braces won't achieve high enough precedence for surrounding expressional or higher production\n")
                      ("Node: ")(node)("\n")
                      ("Surr prod: %d node prod: %d", (int)surround_prod, (int)node_prod); 
			}
			if( do_init )
				s += "\n";
            if( do_boot )
                s += "{ // RIP-stmt\n";

            if( do_boot || semicolon )
				surround_prod = Syntax::Production::BOOT_STATEMENT;
			s += Dispatch( kit, node, surround_prod );
			
            if( semicolon )
                s += "; // RIP-stmt\n";
            if( do_boot )
                s += "} // RIP-stmt\n";            
            break;
        }

        case Syntax::Production::BOOT_EXPR...Syntax::Production::TOP_EXPR: // Expression productions at different precedences
        {
            // If current production has too-high precedence, boot back down using parentheses
            if( do_boot )
				ASSERT( Syntax::GetPrecedence(surround_prod) <= Syntax::GetPrecedence(Syntax::Production::PARENTHESISED) )
					  ("Parentheses won't achieve high enough precedence for surrounding production\n")
					  ("Node: ")(node)("\n")
					  ("Surr prod: %d node prod: %d", (int)surround_prod, (int)node_prod); 
					  
			// Deal with expression in initialiser production by prepending =
			if( do_init )
				s += " = // RIP-expr "+Trace(node)+"\n";
            if( do_boot )
                s += "(";

            if( do_boot || semicolon )
				surround_prod = Syntax::Production::BOOT_EXPR;
			s += Dispatch( kit, node, surround_prod );

            if( do_boot )
                s += ")";            
            if( semicolon )
                s += "; // RIP-expr \n";
            break;
        }
        
        default: 
        {
            if( semicolon )
				surround_prod = Syntax::Production::BOOT_EXPR;
			s += Dispatch( kit, node, surround_prod );
			
            if( semicolon )
                s += "; // RIP-default \n";            
			break;
		}
    }
    
    return s;
}


string Render::Dispatch( const Render::Kit &kit, TreePtr<Node> node, Syntax::Production surround_prod )
{
    if( TreePtr<Uninitialised>::DynamicCast(node) )
        return string();  
    else if( auto program = TreePtr<Program>::DynamicCast(node) )
        return RenderProgram( kit, program, surround_prod );
    else if( auto identifier = TreePtr<Identifier>::DynamicCast(node) ) // Identifier can be a kind of type or expression
        return RenderIdentifier( kit, identifier, surround_prod );
    else if( auto access = TreePtr<AccessSpec>::DynamicCast(node) ) // Identifier can be a kind of type or expression
        return RenderAccessSpec( kit, access, surround_prod );
    else if( auto floating = TreePtr<Floating>::DynamicCast(node) )
        return RenderFloating( kit, floating, surround_prod );
    else if( auto integral = TreePtr<Integral>::DynamicCast(node) )
        return RenderIntegral( kit, integral, surround_prod );
    else if( auto type = TreePtr<Type>::DynamicCast(node) )  // Type is a kind of Operator
        return RenderType( kit, type, surround_prod );
    else if( auto literal = DynamicTreePtrCast< Literal >(node) )
        return RenderLiteral( kit, literal, surround_prod );
    else if( auto make_rec = TreePtr<RecordLiteral>::DynamicCast(node) )
        return RenderMakeRecord( kit, make_rec, surround_prod );
    else if( auto call = TreePtr<Call>::DynamicCast(node) )
        return RenderCall( kit, call, surround_prod );
    else if( auto ext_call = TreePtr<SeqArgsCall>::DynamicCast(node) )
        return RenderExteriorCall( kit, ext_call, surround_prod );
    else if( auto macro_decl = TreePtr<MacroDeclaration>::DynamicCast(node) )
        return RenderMacroDeclaration( kit, macro_decl, surround_prod );
    else if( auto macro_stmt = TreePtr<MacroStatement>::DynamicCast(node) )
        return RenderMacroStatement( kit, macro_stmt, surround_prod );
    else if( auto op = TreePtr<Operator>::DynamicCast(node) ) // Operator is a kind of Expression
        return RenderOperator( kit, op, surround_prod );
    else if( auto expression = TreePtr<Expression>::DynamicCast(node) ) // Expression is a kind of Statement
        return RenderExpression( kit, expression, surround_prod );
    else if( auto instance = TreePtr<Instance>::DynamicCast(node) )    // Instance is a kind of Statement and Declaration
        return RenderInstance( kit, instance, surround_prod ); 
    else if( auto ppd = TreePtr<PreProcDecl>::DynamicCast(node) )
        return RenderPreProcDecl(kit, ppd, surround_prod); 
    else if( auto declaration = TreePtr<Declaration>::DynamicCast(node) )
        return RenderDeclaration( kit, declaration, surround_prod );
    else if( auto statement = TreePtr<Statement>::DynamicCast(node) )
        return RenderStatement( kit, statement, surround_prod );
    else
        return ERROR_UNSUPPORTED( node );       
}

string Render::RenderProgram( const Render::Kit &kit, TreePtr<CPPTree::Program> program, Syntax::Production surround_prod )
{
	(void)surround_prod;
    string s;

    AutoPush< TreePtr<Node> > cs( scope_stack, program );

    // Track scopes for name resolution
    s += RenderDeclScope( kit, program ); // gets the .hpp stuff directly 
    
    s += "// Definitions\n";    
    
    // These are rendered here, inside program scope but outside any additional scopes
    // that were on the scope stack when the instance was seen.
    while( !definitions.empty() )
    {
        s += "\n" + RenderInstance( kit, definitions.front(), Syntax::Production::DEFINITION ); // these could go in a .cpp file
        definitions.pop();
    }
    return s;  
}


string Render::RenderLiteral( const Render::Kit &kit, TreePtr<Literal> sp, Syntax::Production surround_prod ) try
{
	(void)surround_prod;
    (void)kit;
    return Sanitise( sp->GetToken() );
}
DEFAULT_CATCH_CLAUSE


string Render::RenderPureIdentifier( const Render::Kit &kit, TreePtr<Identifier> id, Syntax::Production surround_prod ) try
{   
    (void)kit;
    (void)surround_prod;
    string ids;
    if( id )
    {
        if( TreePtr<SpecificIdentifier> ii = DynamicTreePtrCast<SpecificIdentifier>( id ) )
        {           
            if( unique_ids.count(ii) == 0 )
            {
                return ERROR_UNKNOWN( SSPrintf("identifier %s missing from unique_ids", ii->GetToken().c_str() ) );
            }
            ids = unique_ids.at(ii);
        }
        else
            return ERROR_UNSUPPORTED( (id) );

        TRACE( "%s\n", ids.c_str() );
    }
    else
    {
        TRACE();
    }
    ASSERT(ids.size()>0)(*id)(" rendered to an empty string\n");
    return ids;
}
DEFAULT_CATCH_CLAUSE


string Render::ScopeResolvingPrefix( const Render::Kit &kit, TreePtr<Identifier> id, Syntax::Production surround_prod ) try
{
    TreePtr<Node> scope = TryGetScope(id);
       
    //TRACE("%p %p %p\n", program.get(), scope.get(), scope_stack.top().get() );
    if( !scope )
        return ""; // either we're not in a scope or id is undeclared
    else if( scope == scope_stack.top() )
        return ""; // local scope
    else if( DynamicTreePtrCast<Program>( scope ) )
        return "";
    else if( auto e = DynamicTreePtrCast<Enum>( scope ) ) // <- for enum
        return ScopeResolvingPrefix( kit, e->identifier, surround_prod );    // omit scope for the enum itself
    else if( auto r = DynamicTreePtrCast<Record>( scope ) ) // <- for class, struct, union
        return RenderIdentifier( kit, r->identifier, Syntax::Production::SCOPE_RESOLVE ) + "::";
    else if( DynamicTreePtrCast<CallableParams>( scope ) ||  // <- this is for params
             DynamicTreePtrCast<Compound>( scope ) ||    // <- this is for locals in body
             DynamicTreePtrCast<StatementExpression>( scope ) )    // <- this is for locals in body
        return "";
    else
        return ERROR_UNSUPPORTED( scope );
}
DEFAULT_CATCH_CLAUSE


string Render::RenderIdentifier( const Render::Kit &kit, TreePtr<Identifier> id, Syntax::Production surround_prod ) try
{
    // Slight cheat for expediency: below SCOPE_RESOLVE we prepend && which make it expressional
    if( DynamicTreePtrCast< SpecificLabelIdentifier >(id) && surround_prod < Syntax::Production::SCOPE_RESOLVE )
    {
        string s = "&&"; // label-as-variable (GCC extension)  
        return s + RenderIntoProduction( kit, id, Syntax::Production::SCOPE_RESOLVE ); // recurse at strictly higher precedence
    }

    // Slight cheat for expediency: if a PURE_IDENTIFIER is expected, suppress scope resolution.
    // This could lead to the rendering of identifiers in the wrong scope. But, most PURE_IDENTIFIER
    // uses are declaring the id, or otherwise can't cope with the :: anyway. 
    if( surround_prod < Syntax::Production::PURE_IDENTIFIER ) 
    {
        string s = ScopeResolvingPrefix( kit, id, surround_prod );   
        return s + RenderIntoProduction( kit, id, Syntax::Production::PURE_IDENTIFIER ); // recurse at strictly higher precedence
    }
                                     
    return RenderPureIdentifier( kit, id, surround_prod );
}
DEFAULT_CATCH_CLAUSE


string Render::RenderIntegral( const Render::Kit &kit, TreePtr<Integral> type, Syntax::Production surround_prod ) try
{
	(void)surround_prod;
    (void)kit;
    bool ds;
    int64_t width;
    auto ic = DynamicTreePtrCast<SpecificInteger>( type->width );
    ASSERT(ic)("width must be integer");
    width = ic->GetInt64();

    TRACE("width %" PRId64 "\n", width);

    if( width == TypeDb::char_bits )
        ds = TypeDb::char_default_signed;
    else
        ds = true;

    // Produce signed or unsigned if required
    // Note: literal strings can be converted to char * but not unsigned char * or signed char *
    string s;
    if( DynamicTreePtrCast< Signed >(type) && !ds )
        s = "signed ";
    else if( DynamicTreePtrCast< Unsigned >(type) && ds )
        s = "unsigned ";

    // Fix the width
    if( width <= TypeDb::char_bits )
        s += "char";
    else if( width <= TypeDb::integral_bits[clang::DeclSpec::TSW_short] )
        s += "short";
    else if( width <= TypeDb::integral_bits[clang::DeclSpec::TSW_unspecified] )
        s += "int";
    else if( width <= TypeDb::integral_bits[clang::DeclSpec::TSW_long] )
        s += "long";
    else if( width <= TypeDb::integral_bits[clang::DeclSpec::TSW_longlong] )
        s += "long long";
    else
		ASSERTFAIL();

    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderIntegralTypeAndDeclarator( const Render::Kit &kit, TreePtr<Integral> type, string declarator ) try
{
    (void)kit;
    // This function only exists to provide bitfields in member declarations that use declarators.
    // RenderIntegral() can provide the pure types directly, without bitfields.
    ASSERT(!declarator.empty())("I don't think bitfields can be used in anonymous types");

    int64_t width;
    auto ic = DynamicTreePtrCast<SpecificInteger>( type->width );
    ASSERT(ic)("width must be integer");
    width = ic->GetInt64();

    TRACE("width %" PRId64 "\n", width);

    string s = RenderIntoProduction( kit, type, Syntax::Production::SPACE_SEP_DECLARATION );

    s += " " + declarator;

    // Fix the width
    bool bitfield = !( width == TypeDb::char_bits ||
                       width == TypeDb::integral_bits[clang::DeclSpec::TSW_short] ||
                       width == TypeDb::integral_bits[clang::DeclSpec::TSW_unspecified] ||
                       width == TypeDb::integral_bits[clang::DeclSpec::TSW_long] ||
                       width == TypeDb::integral_bits[clang::DeclSpec::TSW_longlong] );

    if( bitfield )
    {
       char b[100];
       sprintf(b, ":%" PRId64, width);
       s += b;
    }

    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderFloating( const Render::Kit &kit, TreePtr<Floating> type, Syntax::Production surround_prod ) try
{
	(void)surround_prod;
    (void)kit;
    string s;
    TreePtr<SpecificFloatSemantics> sem = DynamicTreePtrCast<SpecificFloatSemantics>(type->semantics);
    ASSERT(sem);

    if( &(const llvm::fltSemantics &)*sem == TypeDb::float_semantics )
        s += "float";
    else if( &(const llvm::fltSemantics &)*sem == TypeDb::double_semantics )
        s += "double";
    else if( &(const llvm::fltSemantics &)*sem == TypeDb::long_double_semantics )
        s += "long double";
    else
        ASSERT(0)("no builtin floating type has required semantics"); // TODO drop in a bit vector

    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderTypeAndDeclarator( const Render::Kit &kit, TreePtr<Type> type, string declarator, 
                                        Syntax::Production declarator_prod, Syntax::Production surround_prod, bool constant ) try
{
    // Production passed in here comes from the current value of the delcarator string, not surrounding production.
    Syntax::Production prod_surrounding_declarator = type->GetOperandInDeclaratorProduction();
    ASSERT( prod_surrounding_declarator != Syntax::Production::UNDEFINED )
          ("Rendering type: ")(type)(" in production %d",(int)prod_surrounding_declarator)(" got no surrounding production\n");
    ASSERT( Syntax::GetPrecedence(prod_surrounding_declarator) <= Syntax::GetPrecedence(Syntax::Production::PARENTHESISED) ); // Can't satisfy this production's precedence demand using parentheses
    ASSERT( Syntax::GetPrecedence(declarator_prod) >= Syntax::GetPrecedence(Syntax::Production::BOOT_EXPR) ); // Can't put this node into parentheses
    bool parenthesise = Syntax::GetPrecedence(declarator_prod) < Syntax::GetPrecedence(prod_surrounding_declarator);  
    // Apply to object rather than recursing, because this is declarator    
    bool pure_type = (declarator == "");
    if( parenthesise )
        declarator = "(" + declarator + ")";
    string sdeclarator;
    if( !pure_type )
        sdeclarator = " " + declarator;
    Syntax::Production type_prod = pure_type ? surround_prod  
                                             : Syntax::Production::SPACE_SEP_DECLARATION;
                
    string const_str = constant?"const ":"";

    TRACE();
    if( TreePtr<Integral> i = DynamicTreePtrCast< Integral >(type) )
        return const_str + RenderIntegralTypeAndDeclarator( kit, i, declarator );        
    else if( TreePtr<Constructor> c = DynamicTreePtrCast< Constructor >(type) )
        return declarator + "(" + RenderParams(kit, c) + ")" + const_str;
    else if( TreePtr<Destructor> f = DynamicTreePtrCast< Destructor >(type) )
        return declarator + "()" + const_str;
    else if( TreePtr<Function> f = DynamicTreePtrCast< Function >(type) )
        return RenderTypeAndDeclarator( kit, f->return_type, declarator + "(" + RenderParams(kit, f) + ")" + const_str, 
                                        Syntax::Production::POSTFIX, surround_prod );
    else if( TreePtr<Pointer> p = DynamicTreePtrCast< Pointer >(type) )
        return RenderTypeAndDeclarator( kit, p->destination, string(DynamicTreePtrCast<Const>(p->constancy)?"const ":"") + "*" + const_str + declarator, 
                                        Syntax::Production::PREFIX, surround_prod, false ); // TODO Pointer node to indicate constancy of pointed-to object - would go into this call to RenderTypeAndDeclarator
    else if( TreePtr<Reference> r = DynamicTreePtrCast< Reference >(type) )
        return RenderTypeAndDeclarator( kit, r->destination, string(DynamicTreePtrCast<Const>(p->constancy)?"const ":"") + "&" + const_str + declarator, 
                                        Syntax::Production::PREFIX, surround_prod );
    else if( TreePtr<Array> a = DynamicTreePtrCast< Array >(type) )
        return RenderTypeAndDeclarator( kit, 
                           a->element, 
                           declarator + "[" + RenderIntoProduction(kit, a->size, Syntax::Production::BOOT_EXPR) + "]", 
                           Syntax::Production::POSTFIX,
                           surround_prod,
                           constant );
    else if( TreePtr<SpecificTypeIdentifier> ti = DynamicTreePtrCast< SpecificTypeIdentifier >(type) )
        return const_str + RenderIntoProduction(kit, ti, type_prod) + sdeclarator;
    else if( dynamic_pointer_cast<Labeley>(type) )
        return const_str + "void *" + declarator;
    else // Assume the type renders expressionally
        return const_str + RenderIntoProduction( kit, type, type_prod ) + sdeclarator;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderType( const Render::Kit &kit, TreePtr<CPPTree::Type> type, Syntax::Production surround_prod )
{
	if( DynamicTreePtrCast< Void >(type) )
        return "void";
    else if( DynamicTreePtrCast< Boolean >(type) )
        return "bool";
    	
    // If we got here, we should not be looking at a type that renders expressionally
	if( Syntax::GetPrecedence(type->GetMyProduction()) < Syntax::GetPrecedence(Syntax::Production::BOOT_EXPR) ) 
	{
		// Production ANONYMOUS relates to the fact that we've provided an empty string for the initial declarator.
		return RenderTypeAndDeclarator( kit, type, "", Syntax::Production::ANONYMOUS, surround_prod, false ); 
	}
	else
		return ERROR_UNSUPPORTED(type);
}

// Insert escapes into a string so it can be put in source code
// TODO use \n \r etc and let printable ascii through
string Render::Sanitise( string s ) try
{
    string o;
    for( string::size_type i=0; i<s.size(); i++ )
    {
        char c[10];
        if( s[i] < ' ' )
            o += SSPrintf( c, "\\x%02x", s[i] );
        else
            o += s[i];
    }
    return o;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderOperator( const Render::Kit &kit, TreePtr<Operator> op, Syntax::Production surround_prod ) try
{
    (void)surround_prod;
    ASSERT(op);
    
    string s;
    Sequence<Expression> operands;
    if( auto n = DynamicTreePtrCast< New >(op) )
        return string (DynamicTreePtrCast<Global>(n->global) ? "::" : "") +
               "new(" + RenderOperandSequence( kit, n->placement_arguments ) + ") " +
               RenderIntoProduction( kit, n->type, Syntax::Production::TYPE_IN_NEW ) +
               (n->constructor_arguments.empty() ? "" : "(" + RenderOperandSequence( kit, n->constructor_arguments ) + ")" );
    else if( auto d = DynamicTreePtrCast< Delete >(op) )
        return string(DynamicTreePtrCast<Global>(d->global) ? "::" : "") +
               "delete" +
               (DynamicTreePtrCast<DeleteArray>(d->array) ? "[]" : "") +
               " " + RenderIntoProduction( kit, d->pointer, Syntax::Production::PREFIX );
    else if( auto lu = DynamicTreePtrCast< Lookup >(op) )
        return RenderIntoProduction( kit, lu->object, Syntax::Production::POSTFIX ) + "." +
               RenderIntoProduction( kit, lu->member, Syntax::BoostPrecedence(Syntax::Production::POSTFIX) );
    else if( auto c = DynamicTreePtrCast< Cast >(op) )
        return "(" + RenderIntoProduction( kit, c->type, Syntax::Production::BOOT_EXPR ) + ")" +
               RenderIntoProduction( kit, c->operand, Syntax::Production::PREFIX );
    else if( auto condo = DynamicTreePtrCast< ConditionalOperator >(op) )
    {
        return RenderIntoProduction( kit, condo->condition, Syntax::BoostPrecedence(Syntax::Production::CONDITIONAL) ) + 
               " ? " +
               // Middle expression boots parser - so you can't split it up using (), [] etc
               RenderIntoProduction( kit, condo->expr_then, Syntax::Production::BOOT_EXPR ) + 
               " : " +
               RenderIntoProduction( kit, condo->expr_else, Syntax::Production::CONDITIONAL );          
    }
    else if( auto subs = DynamicTreePtrCast< Subscript >(op) )
    {
        return RenderIntoProduction( kit, subs->destination, Syntax::Production::POSTFIX ) + 
               "[" +
			   RenderIntoProduction( kit, subs->index, Syntax::Production::BOOT_EXPR ) + 
			   "]";
    }
    else if( auto al = DynamicTreePtrCast< ArrayLiteral >(op) )    
        return RenderOperandSequence( kit, al->operands );
    else if( DynamicTreePtrCast< This >(op) )
        return "this";
    else if( auto nco = DynamicTreePtrCast< NonCommutativeOperator >(op) )
        operands = nco->operands;           
    else if( auto co = DynamicTreePtrCast< CommutativeOperator >(op) )
    {
        Sequence<Expression> seq_operands;
        // Operands are in collection, so sort them and put them in a sequence
        for( TreePtr<Node> o : sc.GetTreePtrOrdering(co->operands) )
            operands.push_back( TreePtr<Expression>::DynamicCast(o) );
    }
    else
    {
        return ERROR_UNSUPPORTED(op);
	}

    // Regular operators: kinds of either NonCommutativeOperator or CommutativeOperator; operands in operands  
    if( false )
    {
	}        
#define INFIX(TOK, TEXT, NODE_SHAPED, BASE, CAT, PROD, ASSOC) \
    else if( DynamicTreePtrCast<NODE_SHAPED>(op) ) \
    { \
        Syntax::Production prod_left = Syntax::Production::PROD; \
        Syntax::Production prod_right = Syntax::Production::PROD; \
        switch( Syntax::Association::ASSOC ) \
        { \
            case Syntax::Association::RIGHT: prod_left = Syntax::BoostPrecedence(prod_left); break; \
            case Syntax::Association::LEFT:  prod_right = Syntax::BoostPrecedence(prod_right); break; \
        } \
		Sequence<Expression>::iterator operands_it = operands.begin(); \
        s = RenderIntoProduction( kit, *operands_it, prod_left ); \
        s += TEXT; \
        ++operands_it; \
        s += RenderIntoProduction( kit, *operands_it, prod_right ); \
    }
#define PREFIX(TOK, TEXT, NODE_SHAPED, BASE, CAT, PROD, ASSOC) \
    else if( DynamicTreePtrCast<NODE_SHAPED>(op) ) \
    { \
		Sequence<Expression>::iterator operands_it = operands.begin(); \
        s = TEXT; \
        bool paren = false; \
        /* Prevent interpretation as a member function pointer literal */ \
        if( auto ao = TreePtr<AddressOf>::DynamicCast(op) ) \
            if( auto id = TreePtr<Identifier>::DynamicCast(*operands_it) ) \
                paren = !ScopeResolvingPrefix( kit, id, Syntax::Production::PROD ).empty(); \
        s += (paren?"(":"") + RenderIntoProduction( kit, *operands_it, Syntax::Production::PROD) + (paren?")":""); \
    }
#define POSTFIX(TOK, TEXT, NODE_SHAPED, BASE, CAT, PROD, ASSOC) \
    else if( DynamicTreePtrCast<NODE_SHAPED>(op) ) \
    { \
		Sequence<Expression>::iterator operands_it = operands.begin(); \
        s = RenderIntoProduction( kit, *operands_it, Syntax::Production::PROD ); \
        s += TEXT; \
    }
#include "tree/operator_data.inc"
    else
    {
        s = ERROR_UNSUPPORTED(op);
    }
    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderCallArgs( const Render::Kit &kit, TreePtr<Call> call ) try
{   
    // If CallableParams, generate some arguments, resolving the order using the original function type
    TreePtr<Node> ctype = TypeOf::instance.Get(kit, call->callee).GetTreePtr();
    ASSERT( ctype );

    // Convert f->params from Parameters to Declarations
    Sequence<Declaration> param_sequence;   
    if( auto f = TreePtr<CallableParams>::DynamicCast(ctype) )  
        for( auto param : f->params )
            param_sequence.push_back(param); 

    // Determine args sequence using param sequence
    Sequence<Expression> arg_sequence  = SortMapOperands( call, param_sequence );
    
    // Render to strings
    list<string> ls;
    for( TreePtr<Expression> e : arg_sequence )
        ls.push_back( RenderIntoProduction( kit, e, Syntax::Production::COMMA_SEP ) );

    // Do the syntax
    return Join( ls, ", ", "(", ")" );
}
DEFAULT_CATCH_CLAUSE


string Render::RenderCall( const Render::Kit &kit, TreePtr<Call> call, Syntax::Production surround_prod ) try
{
	(void)surround_prod;
    string s;

    // Render the expression that resolves to the function name unless this is
    // a constructor call in which case just the name of the thing being constructed.
    if( TreePtr<Expression> base = TypeOf::instance.TryGetConstructedExpression( kit, call ).GetTreePtr() )
        s += RenderIntoProduction( kit, base, Syntax::Production::POSTFIX );
    else
        s += RenderIntoProduction( kit, call->callee, Syntax::Production::POSTFIX );

    s += RenderCallArgs(kit, call);
    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderExprSeq( const Render::Kit &kit, Sequence<Expression> seq ) try
{
    list<string> renders;
    for( TreePtr<Expression> e : seq )    
        renders.push_back( RenderIntoProduction(kit, e, Syntax::Production::COMMA_SEP) );               
    return Join(renders, ", ", "(", ")");
}
DEFAULT_CATCH_CLAUSE


string Render::RenderExteriorCall( const Render::Kit &kit, TreePtr<SeqArgsCall> call, Syntax::Production surround_prod ) try
{
	(void)surround_prod;
    string args_in_parens = RenderExprSeq(kit, call->arguments);

    // Constructor case: spot by use of Lookup to empty-named method. Elide the "."
    if( auto lu = DynamicTreePtrCast< Lookup >(call->callee) )
        if( auto id = DynamicTreePtrCast< InstanceIdentifier >(lu->member) )
            if( id->GetToken().empty() )
                return RenderIntoProduction( kit, lu->object, Syntax::Production::POSTFIX ) + args_in_parens;

    // Other funcitons just evaluate
    return RenderIntoProduction( kit, call->callee, Syntax::Production::POSTFIX ) + args_in_parens;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderMacroStatement( const Render::Kit &kit, TreePtr<MacroStatement> ms, Syntax::Production surround_prod ) try
{
	(void)surround_prod;
	string s = RenderIntoProduction( kit, ms->identifier, Syntax::Production::POSTFIX );
	
    list<string> renders; // TODO duplicated code, factor out into RenderSeqMacroArgs()
    for( TreePtr<Node> node : ms->arguments )
        renders.push_back( RenderIntoProduction(kit, node, Syntax::Production::COMMA_SEP) );
    s += Join(renders, ", ", "(", ");\n");
    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderExpression( const Render::Kit &kit, TreePtr<Initialiser> expression, Syntax::Production surround_prod ) try
{
    (void)surround_prod;
      
    if( auto ce = DynamicTreePtrCast< StatementExpression >(expression) )
    {
        string s = "{ ";
        AutoPush< TreePtr<Node> > cs( scope_stack, ce );
        s += RenderDeclScope( kit, ce ); // Must do this first to populate backing list
        for( TreePtr<Statement> st : ce->statements )    
            s += RenderIntoProduction( kit, st, Syntax::Production::STATEMENT_LOW );    
        return s + " }";
    }
    else if( auto pot = DynamicTreePtrCast< SizeOf >(expression) )
        return "sizeof(" + RenderIntoProduction( kit, pot->argument, Syntax::Production::BOOT_EXPR ) + ")";               
    else if( auto pot = DynamicTreePtrCast< AlignOf >(expression) )
        return "alignof(" + RenderIntoProduction( kit, pot->argument, Syntax::Production::BOOT_EXPR ) + ")";    
    else
        return ERROR_UNSUPPORTED(expression);
}
DEFAULT_CATCH_CLAUSE


string Render::RenderMakeRecord( const Render::Kit &kit, TreePtr<RecordLiteral> make_rec, Syntax::Production surround_prod ) try
{
	(void)surround_prod;
    string s;

    // Get the record
    TreePtr<TypeIdentifier> id = DynamicTreePtrCast<TypeIdentifier>(make_rec->type);
    ASSERT(id);
    
    // We have an interesting problem: there are two collections, but an ordering
    // is implied in the generalised case: the record type has a collection of 
    // fields, but they need to be sorted based on dependency order when rendering
    // the records (we also aim for repeatability here). This ordering must then
    // be applied to the IdValueMap in order to get a match-up between sub-expressions
    // and fields. I think C++ side-steps this by diallowing the RecordLiteral syntax
    // in classes where dependencies might matter.

    TreePtr<Record> r = GetRecordDeclaration(kit, id).GetTreePtr();
    // Make sure we have the same ordering as when the record was rendered
    Sequence<Declaration> sorted_members = SortDecls( r->members, true, unique_ids );

    // Determine args sequence using param sequence
    Sequence<Expression> sub_expr_sequence  = SortMapOperands( make_rec, sorted_members );
    
    // Render to strings
    list<string> ls;
    for( TreePtr<Expression> e : sub_expr_sequence )
        ls.push_back( RenderIntoProduction( kit, e, Syntax::Production::COMMA_SEP ) );

    // Do the syntax
    s += "(" + RenderIntoProduction( kit, make_rec->type, Syntax::Production::BOOT_EXPR ) + ")"; 
    s += Join( ls, ", ", "{", "}" );   // Use of {} in expressions is irregular so handle locally 
    return s;
}
DEFAULT_CATCH_CLAUSE


Sequence<Expression> Render::SortMapOperands( TreePtr<IdValueMap> ro,
                                              Sequence<Declaration> key_sequence )
{   
    Sequence<Expression> out_sequence;
    for( TreePtr<Declaration> d : key_sequence )
    {
        // We only care about instances...
        if( TreePtr<Instance> i = DynamicTreePtrCast<Instance>( d ) )
        {
            // ...and not function instances
            if( !DynamicTreePtrCast<Callable>( i->type ) )
            {
                // search init for matching member (not a map: this is a Collection of IdValuePair nodes)
                for( TreePtr<IdValuePair> mi : ro->operands )
                {
                    if( i->identifier == mi->key )
                    {
                        out_sequence.push_back( mi->value );
                        break;
                    }
                }
            }
        }
    }
    return out_sequence;
}


string Render::RenderAccessSpec( const Render::Kit &kit, TreePtr<AccessSpec> access, Syntax::Production surround_prod ) try
{
	(void)surround_prod;
    (void)kit;
    if( DynamicTreePtrCast<Public>( access ) )
        return "public";
    else if( DynamicTreePtrCast<Private>( access ) )
        return "private";
    else if( DynamicTreePtrCast<Protected>( access ) )
        return "protected";
    else
        return ERROR_UNKNOWN("access spec");
}
DEFAULT_CATCH_CLAUSE


string Render::RenderStorage( const Render::Kit &kit, TreePtr<Instance> st ) try
{
    (void)kit;
    if( DynamicTreePtrCast<Program>( scope_stack.top() ) )
        return ""; // at top-level scope, everything is set to static, but don't actually output the word
    else if( DynamicTreePtrCast<Static>( st ) )
        return "static ";
    else if( DynamicTreePtrCast<LocalVariable>( st ) )
        return ""; // Assume automatic allocation is the default
    else if( DynamicTreePtrCast<Temporary>( st ) )
        return "/*temp*/ ";
    else if( TreePtr<Field> no = DynamicTreePtrCast<Field>( st ) )
    {
        TreePtr<Virtuality> v = no->virt;
        if( DynamicTreePtrCast<Virtual>( v ) )
            return "virtual ";
        else if( DynamicTreePtrCast<NonVirtual>( v ) )
            return "";
        else
            return ERROR_UNKNOWN("virtualness");
    }
    else
        return ERROR_UNKNOWN(st->GetTypeName());
}
DEFAULT_CATCH_CLAUSE


void Render::ExtractInits( const Render::Kit &kit, 
                           Sequence<Statement> &body, 
                           Sequence<Statement> &inits, 
                           Sequence<Statement> &remainder )
{
    // Initialisers are just calls to the constructor embedded in the body. In Inferno,
    // we call a constructor by 
    for( TreePtr<Statement> s : body )
    {
        if( auto call = DynamicTreePtrCast< GoSub >(s) )
        {
            try
            {
                if( TypeOf::instance.TryGetConstructedExpression( kit, call ) )
                {
                    inits.push_back(s);
                    continue;
                }
            }
            catch( ::Mismatch &me )
            {
                remainder.push_back(MakeTreeNode<SpecificString>(RenderMismatchException(__func__, me)));
                continue;
            }
        }
        remainder.push_back(s);
    }
}


string Render::RenderInstanceProto( const Render::Kit &kit, 
                                    TreePtr<Instance> o ) try
{
    string s;
    bool constant=false;

    ASSERT(o->type);

    if( TreePtr<Static> st = DynamicTreePtrCast<Static>(o) )
        if( DynamicTreePtrCast<Const>(st->constancy) )
            constant = true;
    if( TreePtr<Field> f = DynamicTreePtrCast<Field>(o) )
        if( DynamicTreePtrCast<Const>(f->constancy) )
            constant = true;
            
    string name;     
    Syntax::Production starting_declarator_prod; 
    TreePtr<Constructor> con = DynamicTreePtrCast<Constructor>(o->type);
    TreePtr<Destructor> de = DynamicTreePtrCast<Destructor>(o->type);
    if( con || de )
    {
		name = ScopeResolvingPrefix(kit, o->identifier, Syntax::Production::SCOPE_RESOLVE);
        // TODO use GetRecordDeclaration( Typeof( o->identifier ) ) and leave scopes out of it
        TreePtr<Record> rec = DynamicTreePtrCast<Record>( TryGetScope( o->identifier ) );
        ASSERT( rec );        
        name += (de ? "~" : ""); 
        starting_declarator_prod = Syntax::Production::PURE_IDENTIFIER; // we already rendered the scope prefix into name
        name += RenderIntoProduction(kit, rec->identifier, starting_declarator_prod);
    }
    else
    {
        starting_declarator_prod = Syntax::Production::SCOPE_RESOLVE;
        name += RenderIntoProduction(kit, o->identifier, starting_declarator_prod);
    }

    s += RenderTypeAndDeclarator( kit, o->type, name, starting_declarator_prod, Syntax::Production::PROTOTYPE, constant );

    return s;
} 
DEFAULT_CATCH_CLAUSE


bool Render::IsDeclared( const Render::Kit &kit, TreePtr<Identifier> id )
{
    try
    {
        DeclarationOf().TryApplyTransformation( kit, id );
        return true;
    }
    catch(DeclarationOf::DeclarationNotFound &)
    {
        return false;               
    }
}   


string Render::RenderInitialisation( const Render::Kit &kit, TreePtr<Initialiser> init ) try
{
	string s;
	if( TreePtr<Expression> ei = DynamicTreePtrCast<Expression>( init ) )
    {
        // Attempt direct initialisation by providing args for a constructor call
        if( auto call = DynamicTreePtrCast<SeqArgsCall>( ei ) )
        {
            if( auto lu = TreePtr<Lookup>::DynamicCast(call->callee) )
                if( auto id = TreePtr<InstanceIdentifier>::DynamicCast(lu->member) )
                    if( id->GetToken().empty() ) // syscall to a nameless member function => sys construct
                        return RenderExprSeq(kit, call->arguments) + ";\n";
        }
        if( auto call = DynamicTreePtrCast<Call>( ei ) ) try
        {       
            if( TypeOf::instance.TryGetConstructedExpression( kit, call ).GetTreePtr() )        
                return RenderCallArgs(kit, call) + ";\n";
        }
        catch(DeclarationOf::DeclarationNotFound &)
        {
        }   
    }
    else if( auto stmt = DynamicTreePtrCast<Statement>(init) )
    {
        // Put the contents of the body into a Compound-like form even if there's only one
        // Statement there - this is because we will wrangle with them later
        Sequence<Statement> code;
        Collection<Declaration> members;
        if( TreePtr<Compound> comp = DynamicTreePtrCast<Compound>(stmt) )
        {
            members = comp->members;
            code = comp->statements;
        }
        else 
            code.push_back( stmt );

        // Seperate the statements into constructor initialisers and "other stuff"
        Sequence<Statement> inits;
        Sequence<Statement> remainder;
        ExtractInits( kit, code, inits, remainder );

        // Render the constructor initialisers if there are any
        if( !inits.empty() )
        {
            s += " :\n";
            s += RenderConstructorInitList( kit, inits );

			// Render the other stuff as a Compound so we always get {} in all cases
			auto r = MakeTreeNode<Compound>();
			r->members = members;
			r->statements = remainder;		
			init = r;	
		}	
    }

    return s + RenderIntoProduction(kit, init, Syntax::Production::INITIALISER); 
}
DEFAULT_CATCH_CLAUSE


string Render::RenderInstance( const Render::Kit &kit, TreePtr<Instance> o, Syntax::Production surround_prod )
{
    string s;
       
	switch( surround_prod )
	{
		case Syntax::Production::DEFINITION:
			{
				// Definition is out-of-line so skip the storage
				s += RenderInstanceProto( kit, o );
				AutoPush< TreePtr<Node> > cs( scope_stack, TryGetScope( o->identifier ) );
				s += RenderInitialisation( kit, o->initialiser );	
				s += "\n";		
			}
			break;
			
		default: // should be one of the statement-level prods
			s += RenderStorage(kit, o);
			s += RenderInstanceProto( kit, o );
		    if( ShouldSplitInstance(kit, o) )
			{
				// Emit just a prototype now and request definition later
				s += "; // RI-ssi\n";
				// Split out the definition of the instance for rendering later at Program scope
				definitions.push(o);
			}		
			else
			{
				// Emit the whole lot in-line
				AutoPush< TreePtr<Node> > cs( scope_stack, TryGetScope( o->identifier ) );
				s += RenderInitialisation( kit, o->initialiser );							
				s += "\n";		
			}
	}
	return s;
}


// Non-const static objects in records and functions 
// get split into a part that goes into the record (main line of rendering) and
// a part that goes separately (definitions gets appended at the very end).
// Do all functions, since SortDecls() ignores function bodies for dep analysis
bool Render::ShouldSplitInstance( const Render::Kit &kit, TreePtr<Instance> o ) 
{
    (void)kit;
    if( DynamicTreePtrCast<Callable>( o->type ) )
    {
        // ----- functions -----
        if( auto smf = TreePtr<MacroDeclaration>::DynamicCast(o) )
            return false; // don't split these
            
        return true;
    }
    else
    {
        // ----- objects ------ 
        if( !DynamicTreePtrCast<Record>( scope_stack.top() ) )
            return false;
        
        if( TreePtr<Static> s = DynamicTreePtrCast<Static>(o) )
        {
            if( DynamicTreePtrCast<Const>(s->constancy) && DynamicTreePtrCast<Numeric>( o->type ) )
                return false;

            return true;                
        }

        return false;
    }
}


string Render::RenderMacroDeclaration( const Render::Kit &kit, TreePtr<MacroDeclaration> md, Syntax::Production surround_prod )
{
	(void)surround_prod;	
    // ---- Proto ----
	string s = RenderIntoProduction( kit, md->identifier, Syntax::Production::POSTFIX );
	list<string> renders;
	for( TreePtr<Node> node : md->arguments )
		renders.push_back( RenderIntoProduction(kit, node, Syntax::Production::COMMA_SEP) );
	s += Join(renders, ", ", "(", ")");
	
	// ---- Initialisation ----	    
    return s + RenderInitialisation( kit, md->initialiser );
}


string Render::RenderRecordProto( const Render::Kit &kit, TreePtr<Record> record )
{
    string s;
    if( DynamicTreePtrCast< Class >(record) )
        s += "class";
    else if( DynamicTreePtrCast< Struct >(record) )
        s += "struct";
    else if( DynamicTreePtrCast< Union >(record) )
        s += "union";
    else if( DynamicTreePtrCast< Enum >(record) )
        s += "enum";
    else
        return ERROR_UNSUPPORTED(record);

    // Name of the record
    s += " " + RenderIntoProduction(kit, record->identifier, Syntax::Production::SPACE_SEP_DECLARATION);
    
    return s;
}


string Render::RenderPreProcDecl(const Render::Kit &kit, TreePtr<PreProcDecl> ppd, Syntax::Production surround_prod ) try
{
	(void)surround_prod;
    (void)kit;
    if( auto si = TreePtr<SystemInclude>::DynamicCast(ppd) )
        return "#include <" + si->filename->GetString() + ">";
    else if( auto si = TreePtr<LocalInclude>::DynamicCast(ppd) )
        return "#include " + si->filename->GetToken();
    else
        return ERROR_UNSUPPORTED(ppd);     
}
DEFAULT_CATCH_CLAUSE


string Render::RenderDeclaration( const Render::Kit &kit, TreePtr<Declaration> declaration, Syntax::Production surround_prod ) try
{
    TRACE();
    string s;
	(void)surround_prod;

    if( TreePtr<Typedef> t = DynamicTreePtrCast< Typedef >(declaration) )
    {
        Syntax::Production starting_declarator_prod = Syntax::Production::PURE_IDENTIFIER;
        auto id = RenderIntoProduction(kit, t->identifier, starting_declarator_prod);
        s += "typedef " + RenderTypeAndDeclarator( kit, t->type, id, starting_declarator_prod, Syntax::Production::SPACE_SEP_DECLARATION );
    }
    else if( TreePtr<Record> r = DynamicTreePtrCast< Record >(declaration) )
    {
        // Prototype of the record
        s += RenderRecordProto( kit, r );
        
        if( surround_prod == Syntax::Production::PROTOTYPE )
			return s;

        // Base classes
        if( TreePtr<InheritanceRecord> ir = DynamicTreePtrCast< InheritanceRecord >(declaration) )
        {
            if( !ir->bases.empty() )
            {
                s += " : ";
                bool first=true;
                for( TreePtr<Node> bn : sc.GetTreePtrOrdering(ir->bases) )
                {
                    if( !first )
                        s += ", ";
                    first=false;
                    auto b = TreePtr<Base>::DynamicCast(bn);
                    ASSERT( b );
                    s += RenderIntoProduction(kit, b->access, Syntax::Production::TOKEN ) + " ";
                    s += RenderIntoProduction(kit, b->record, Syntax::Production::SCOPE_RESOLVE);
                }
            }
        }

        // Members
        s += "\n{ // memb\n";
        TreePtr<AccessSpec> a;
        if( DynamicTreePtrCast< Class >(r) )
            a = MakeTreeNode<Private>();
        else if( DynamicTreePtrCast< Struct >(r) )
            a = MakeTreeNode<Public>();
        else if( DynamicTreePtrCast< Union >(r) )
            a = MakeTreeNode<Public>();
        else if( DynamicTreePtrCast< Enum >(r) )
            a = nullptr;
        else
            return ERROR_UNSUPPORTED(declaration);

        AutoPush< TreePtr<Node> > cs( scope_stack, r );
        if( a )
            s += RenderDeclScope( kit, r, a );          
        else
            s += RenderEnumBodyScope( kit, r ); 
            
        s += "};\n";
        // Add blank lines before and after
        s = '\n' + s + '\n';
    }
    else if( TreePtr<Label> l = DynamicTreePtrCast<Label>(declaration) )
        return RenderIntoProduction(kit, l->identifier, Syntax::Production::PURE_IDENTIFIER) + ":;\n"; // need ; after a label in case last in compound block
    else
        s += ERROR_UNSUPPORTED(declaration);

    TRACE();
    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderStatement( const Render::Kit &kit, TreePtr<Statement> statement, Syntax::Production surround_prod ) try
{
    (void)surround_prod;
    TRACE();
    ASSERT( statement );
    //printf( "%s %d things\n", typeid(*statement).name(), statement->Itemise().size() );
    if( TreePtr<Declaration> d = DynamicTreePtrCast< Declaration >(statement) )
        return RenderDeclaration( kit, d, surround_prod );
    else if( TreePtr<Compound> c = DynamicTreePtrCast< Compound >(statement) )
    {
        string s;
        AutoPush< TreePtr<Node> > cs( scope_stack, c );
        s += RenderDeclScope( kit, c ); // Must do this first to populate backing list
        for( TreePtr<Statement> st : c->statements )    
            s += RenderIntoProduction( kit, st, Syntax::Production::STATEMENT_LOW );    
        return s;
    }
    else if( TreePtr<Expression> e = DynamicTreePtrCast< Expression >(statement) )
        return RenderIntoProduction(kit, e, surround_prod);
    else if( TreePtr<Return> es = DynamicTreePtrCast<Return>(statement) )
        return "return " + RenderIntoProduction(kit, es->return_value, Syntax::Production::SPACE_SEP_STATEMENT);
    else if( TreePtr<Goto> g = DynamicTreePtrCast<Goto>(statement) )
    {
        if( TreePtr<SpecificLabelIdentifier> li = DynamicTreePtrCast< SpecificLabelIdentifier >(g->destination) )
            return "goto " + RenderIntoProduction(kit, li, Syntax::Production::SPACE_SEP_STATEMENT);  // regular goto
        else
            return "goto *" + RenderIntoProduction(kit, g->destination, Syntax::Production::PREFIX); // goto-a-variable (GCC extension)
    }
    else if( TreePtr<If> i = DynamicTreePtrCast<If>(statement) )
    {
        bool has_else_clause = !DynamicTreePtrCast<Nop>(i->body_else); // Nop means no else clause
        string s;
        s += "if( " + RenderIntoProduction(kit, i->condition, Syntax::Production::CONDITION) + " )\n";
        // The choice of production here causes then "else" ambuguity to be resolved.
        s += RenderIntoProduction(kit, i->body, has_else_clause ? Syntax::Production::STATEMENT_HIGH : Syntax::Production::STATEMENT_LOW);
        if( has_else_clause )  
            s += "else\n" + RenderIntoProduction(kit, i->body_else, Syntax::Production::STATEMENT_LOW);
        return s;
    }
    else if( TreePtr<While> w = DynamicTreePtrCast<While>(statement) )
        return "while( " + 
               RenderIntoProduction(kit, w->condition, Syntax::Production::CONDITION) + " )\n" +
               RenderIntoProduction(kit, w->body, surround_prod);
    else if( TreePtr<Do> d = DynamicTreePtrCast<Do>(statement) )
        return "do\n" +
               RenderIntoProduction(kit, d->body, Syntax::Production::STATEMENT_LOW) +
               "while( " + RenderIntoProduction(kit, d->condition, Syntax::Production::CONDITION) + " )";
    else if( TreePtr<For> f = DynamicTreePtrCast<For>(statement) )
        return "for( " + 
               RenderIntoProduction(kit, f->initialisation, Syntax::Production::STATEMENT_LOW) + 
               RenderIntoProduction(kit, f->condition, Syntax::Production::STATEMENT_LOW) + 
               RenderIntoProduction(kit, f->increment, Syntax::Production::BOOT_EXPR) + " )\n" +
               RenderIntoProduction(kit, f->body, surround_prod);
    else if( TreePtr<Switch> s = DynamicTreePtrCast<Switch>(statement) )
        return "switch( " + RenderIntoProduction(kit, s->condition, Syntax::Production::CONDITION) + " )\n" +
               RenderIntoProduction(kit, s->body, surround_prod);
    else if( TreePtr<Case> c = DynamicTreePtrCast<Case>(statement) )
        return "case " + RenderIntoProduction(kit, c->value, Syntax::Production::SPACE_SEP_STATEMENT) + ":";
    else if( TreePtr<RangeCase> rc = DynamicTreePtrCast<RangeCase>(statement) )
        // GCC extension: assume that ... is part of the case statement, and can boot the expressions.
        return "case " + 
               RenderIntoProduction(kit, rc->value_lo, Syntax::Production::SPACE_SEP_STATEMENT) + 
               " ... " + 
               RenderIntoProduction(kit, rc->value_hi, Syntax::Production::SPACE_SEP_STATEMENT) + 
               ":";
    else if( DynamicTreePtrCast<Default>(statement) )
        return "default:";
    else if( DynamicTreePtrCast<Continue>(statement) )
        return "continue";
    else if( DynamicTreePtrCast<Break>(statement) )
        return "break";
    else if( DynamicTreePtrCast<Nop>(statement) )
        return "";
    else
        return ERROR_UNSUPPORTED(statement);
}
DEFAULT_CATCH_CLAUSE


string Render::RenderConstructorInitList( const Render::Kit &kit, 
                                          Sequence<Statement> spe ) try
{
    TRACE();
    string s; 
    bool first = true;
    for( TreePtr<Statement> st : spe )
    {
        if( !first )
            s += ",\n";
        s += "    "; // indentation
        if( auto e = TreePtr<Expression>::DynamicCast(st) )
            s += RenderIntoProduction( kit, e, Syntax::Production::COMMA_SEP );
        else 
            s += ERROR_UNSUPPORTED(st);
        first = false;
    }
    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderEnumBodyScope( const Render::Kit &kit, 
                                    TreePtr<CPPTree::Record> record ) try
{
    TRACE();   
    string s;
    bool first = true;
    for( TreePtr<Declaration> pe : record->members )
    {
        if( !first )
            s += ",\n";
            
        auto o = TreePtr<Instance>::DynamicCast(pe);
        if( !o )
        {
            s += ERROR_UNSUPPORTED(pe);
            continue;
        }
        s += RenderIntoProduction(kit, o->identifier, Syntax::BoostPrecedence(Syntax::Production::ASSIGN)) + " = ";
        
        auto ei = TreePtr<Expression>::DynamicCast( o->initialiser );
        if( !ei )
        {
            s += ERROR_UNSUPPORTED(o->initialiser);
            continue;
        }       
        s += RenderIntoProduction(kit, ei, Syntax::Production::ASSIGN);

        first = false;    
    }
    return s + "\n";
}
DEFAULT_CATCH_CLAUSE


string Render::RenderOperandSequence( const Render::Kit &kit, 
                                      Sequence<Expression> spe ) try
{
    TRACE();
    string s;
	list<string> renders;    
    for( TreePtr<Expression> pe : spe )
		renders.push_back( RenderIntoProduction( kit, pe, Syntax::Production::COMMA_SEP ) );
    return Join(renders, ", ", "{", "}"); // Use of {} in expressions is irregular so handle locally 
}
DEFAULT_CATCH_CLAUSE


string Render::MaybeRenderFieldAccess( const Render::Kit &kit, TreePtr<Declaration> declaration,
                                       TreePtr<AccessSpec> *current_access )
{
    ASSERT( current_access );
    
    TreePtr<AccessSpec> this_access = MakeTreeNode<Public>();

    // Decide access spec for this declaration (explicit if instance, otherwise force to Public)
    if( TreePtr<Field> f = DynamicTreePtrCast<Field>(declaration) )
        this_access = f->access;

    return MaybeRenderAccessColon( kit, this_access, current_access );  
}                                 


string Render::MaybeRenderAccessColon( const Render::Kit &kit, TreePtr<AccessSpec> this_access,
                                       TreePtr<AccessSpec> *current_access )
{
    ASSERT( current_access );

    // Now decide whether we actually need to render an access spec (ie has it changed?)
    if( typeid(*this_access) != typeid(**current_access) ) // current_access spec must have changed
    {
        *current_access = this_access;
        return RenderIntoProduction( kit, this_access, Syntax::Production::TOKEN ) + ":\n";
    }
    
    return "";  
}                                 


string Render::RenderDeclScope( const Render::Kit &kit, 
                                TreePtr<DeclScope> decl_scope,
                                TreePtr<AccessSpec> init_access ) try
{
    TRACE();
    Sequence<Declaration> sorted = SortDecls( decl_scope->members, true, unique_ids );

    // Emit a prototype for each record and preproc
    string s;
    for( TreePtr<Declaration> pd : sorted ) //for( int i=0; i<sorted.size(); i++ )
    {       
        if( auto ppd = DynamicTreePtrCast<PreProcDecl>(pd) )
        {
            s += RenderIntoProduction( kit, ppd, Syntax::Production::DECLARATION ) + "\n";
            continue;
        }
        
        TreePtr<Record> r = DynamicTreePtrCast<Record>(pd); 
        if( !r )
            continue; // only do records and preprocessor decls
            
        if( DynamicTreePtrCast<Enum>(r) ) 
            continue; // but not an enum
    
        if( init_access )
            s += MaybeRenderFieldAccess( kit, r, &init_access );
        //s += RenderRecordProto( kit, r ) + "; // RDS-record proto\n";   
        s += RenderIntoProduction( kit, r, Syntax::Production::PROTOTYPE ) + "; // RDS-record proto (new)\n"; 
    }
    
    // Emit the actual definitions, sorted for dependencies
    for( TreePtr<Declaration> d : sorted )
    {
        if( DynamicTreePtrCast<PreProcDecl>(d) )
            continue;
            
        if( init_access )
            s += MaybeRenderFieldAccess( kit, d, &init_access );        
        s += RenderIntoProduction( kit, d, Syntax::Production::STATEMENT_LOW );
    }
    TRACE();
    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderParams( const Render::Kit &kit, 
                             TreePtr<CallableParams> key ) try
{
    TRACE();
    Sequence<Declaration> sorted;
    for( auto param : key->params )
        sorted.push_back(param); // no sorting required
                
    string s;   
    bool first = true;
    for( TreePtr<Declaration> d : sorted )
    {
        if( !first )
            s += ", ";
        
        auto o = TreePtr<Instance>::DynamicCast(d);
        if( !o )
        {
            s += ERROR_UNSUPPORTED(d);
            continue;
        }
        Syntax::Production starting_declarator_prod = Syntax::Production::PURE_IDENTIFIER;
        string name = RenderIntoProduction(kit, o->identifier, starting_declarator_prod);
        s += RenderTypeAndDeclarator( kit, o->type, name, starting_declarator_prod, Syntax::Production::CONDITION, false );
            
        first = false;
    }
    return s;
}
DEFAULT_CATCH_CLAUSE


TreePtr<Scope> Render::TryGetScope( TreePtr<Identifier> id )
{
    if( scope_stack.empty() ) 
        return nullptr; // We aren't even in any scopes

    try
    {
        return GetScope( context, id );
    }
    catch( ScopeNotFoundMismatch & )
    {
        // There is a scope but our id us not in it, maybe it was undeclared?
        return nullptr;
    }
}


string Render::RenderMismatchException( string fname, const Mismatch &me )
{
    return "«"+fname+"() error: "+me.What()+"»";
}
