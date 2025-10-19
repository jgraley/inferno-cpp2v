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
#include "vn/lang/sort_decls.hpp"
#include "clang/Parse/DeclSpec.h"
#include "vn/lang/uniquify_identifiers.hpp"
#include "cpprender.hpp"

using namespace std;
using namespace CPPTree;

// TODO indent back to previous level at end of string
#define ERROR_UNKNOWN(V) \
    ( string( "\n«" ) + \
      string( V ) + \
      string( " not supported in " ) + \
      string( __func__ ) + \
      string( "()»\n" ) )

#define ERROR_UNSUPPORTED(P) \
    ERROR_UNKNOWN( P ? TYPE_ID_NAME(*P) : "<nullptr>" )

// Don't like the layout of rendered code?
// There's a .clang-format in repo root.


// For #400 make methods that return strings try-functions
// and use this for the catch clause.
#define DEFAULT_CATCH_CLAUSE \
    catch( const ::Mismatch &e ) \
    { \
        return RenderMismatchException( __func__, e ); \
    } 


CppRender::CppRender( string of ) :
	Render( of )
{
}


string CppRender::RenderToString( TreePtr<Node> root )
{       
    // Context is used for various lookups but does not need
    // to be a Scope.
    context = root; 
        
    utils = make_unique<DefaultTransUtils>(context);
    using namespace placeholders;
    kit = VN::RenderKit{ utils.get(),
		                 bind(&Render::RenderIntoProduction, this, _1, _2)  };

    // Make the identifiers unique (does its own tree walk)
    unique_names = UniquifyNames::UniquifyAll( kit, context, false, true );
    
    return kit.render( root, Syntax::Production::PROGRAM );
}



string CppRender::Dispatch( TreePtr<Node> node, Syntax::Production surround_prod )
{
    if( TreePtr<Uninitialised>::DynamicCast(node) )
        return string();  
    else if( auto program = TreePtr<Program>::DynamicCast(node) )
        return RenderProgram( program, surround_prod );
    else if( auto identifier = TreePtr<Identifier>::DynamicCast(node) ) // Identifier can be a kind of type or expression
        return RenderIdentifier( identifier, surround_prod );
    else if( auto access = TreePtr<AccessSpec>::DynamicCast(node) ) // Identifier can be a kind of type or expression
        return RenderAccessSpec( access, surround_prod );
    else if( auto ivp = TreePtr<IdValuePair>::DynamicCast(node) )
        return RenderIdValuePair( ivp, surround_prod );
    else if( auto floating = TreePtr<Floating>::DynamicCast(node) )
        return RenderFloating( floating, surround_prod );
    else if( auto integral = TreePtr<Integral>::DynamicCast(node) )
        return RenderIntegral( integral, surround_prod );
    else if( auto type = TreePtr<Type>::DynamicCast(node) )  // Type is a kind of Operator
        return RenderType( type, surround_prod );
    else if( auto literal = DynamicTreePtrCast< Literal >(node) )
        return RenderLiteral( literal, surround_prod );
    else if( auto call = TreePtr<Call>::DynamicCast(node) )
        return RenderCall( call, surround_prod );
    else if( auto make_rec = TreePtr<RecordLiteral>::DynamicCast(node) )
        return RenderMakeRecord( make_rec, surround_prod );
    else if( auto ext_call = TreePtr<SeqArgsCall>::DynamicCast(node) )
        return RenderExteriorCall( ext_call, surround_prod );
    else if( auto macro_decl = TreePtr<MacroDeclaration>::DynamicCast(node) )
        return RenderMacroDeclaration( macro_decl, surround_prod );
    else if( auto macro_stmt = TreePtr<MacroStatement>::DynamicCast(node) )
        return RenderMacroStatement( macro_stmt, surround_prod );
    else if( auto op = TreePtr<Operator>::DynamicCast(node) ) // Operator is a kind of Expression
        return RenderOperator( op, surround_prod );
    else if( auto expression = TreePtr<Expression>::DynamicCast(node) ) // Expression is a kind of Statement
        return RenderExpression( expression, surround_prod );
    else if( auto instance = TreePtr<Instance>::DynamicCast(node) )    // Instance is a kind of Statement and Declaration
        return RenderInstance( instance, surround_prod ); 
    else if( auto ppd = TreePtr<PreProcDecl>::DynamicCast(node) )
        return RenderPreProcDecl(ppd, surround_prod); 
    else if( auto declaration = TreePtr<Declaration>::DynamicCast(node) )
        return RenderDeclaration( declaration, surround_prod );
    else if( auto statement = TreePtr<Statement>::DynamicCast(node) )
        return RenderStatement( statement, surround_prod );
    else
        return Render::Dispatch( node, surround_prod );       
}


string CppRender::RenderProgram( TreePtr<CPPTree::Program> program, Syntax::Production surround_prod )
{
	(void)surround_prod;
    string s;

    AutoPush< TreePtr<Node> > cs( scope_stack, program );

    // Track scopes for name resolution
    s += RenderDeclScope( program ); // gets the .hpp stuff directly 
    
    s += "// Definitions\n";    
    
    // These are rendered here, inside program scope but outside any additional scopes
    // that were on the scope stack when the instance was seen.
    while( !definitions.empty() )
    {
        s += " " + kit.render( definitions.front(), Syntax::Production::DEFINITION ); // these could go in a .cpp file
        definitions.pop();
    }
    return s;  
}


string CppRender::RenderIdValuePair( TreePtr<IdValuePair> ivp, Syntax::Production surround_prod ) try
{
	// Not part of C/C++ grammer at time of writing but handy for calls with no decl
	(void)surround_prod;
    return kit.render( ivp->key, Syntax::Production::ASSIGN) + 
           "🡆" + 
           kit.render( ivp->value, Syntax::Production::ASSIGN);
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderLiteral( TreePtr<Literal> sp, Syntax::Production surround_prod ) try
{
	(void)surround_prod;
    return Sanitise( sp->GetRenderTerminal() );
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderPureIdentifier( TreePtr<Identifier> id, Syntax::Production surround_prod ) try
{   
    (void)surround_prod;
    string ids;
    if( id )
    {
        if( auto ii = DynamicTreePtrCast<SpecificIdentifier>( id ) )
        {           
            if( unique_names.count(ii) == 0 )
            {
                return ERROR_UNKNOWN( SSPrintf("identifier %s missing from unique_names", ii->GetRenderTerminal().c_str() ) );
            }
            ids = unique_names.at(ii);
        }
        else
            return Render::Dispatch( id, surround_prod );

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


string CppRender::ScopeResolvingPrefix( TreePtr<Identifier> id, Syntax::Production surround_prod ) try
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
        return ScopeResolvingPrefix( e->identifier, surround_prod );    // omit scope for the enum itself
    else if( auto r = DynamicTreePtrCast<Record>( scope ) ) // <- for class, struct, union
        return RenderIdentifier( r->identifier, Syntax::Production::SCOPE_RESOLVE ) + "::";
    else if( DynamicTreePtrCast<CallableParams>( scope ) ||  // <- this is for params
             DynamicTreePtrCast<Compound>( scope ) ||    // <- this is for locals in body
             DynamicTreePtrCast<StatementExpression>( scope ) )    // <- this is for locals in body
        return "";
    else
        return ERROR_UNSUPPORTED( scope );
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderIdentifier( TreePtr<Identifier> id, Syntax::Production surround_prod ) try
{
    // Slight cheat for expediency: below SCOPE_RESOLVE we prepend && which make it expressional
    if( DynamicTreePtrCast< SpecificLabelIdentifier >(id) && surround_prod < Syntax::Production::SCOPE_RESOLVE )
    {
        string s = "&&"; // label-as-variable (GCC extension)  
        return s + kit.render( id, Syntax::Production::SCOPE_RESOLVE ); // recurse at strictly higher precedence
    }

    // Slight cheat for expediency: if a PURE_IDENTIFIER is expected, suppress scope resolution.
    // This could lead to the rendering of identifiers in the wrong scope. But, most PURE_IDENTIFIER
    // uses are declaring the id, or otherwise can't cope with the :: anyway. 
    if( surround_prod < Syntax::Production::PURE_IDENTIFIER ) 
    {
        string s = ScopeResolvingPrefix( id, surround_prod );   
        return s + kit.render( id, Syntax::Production::PURE_IDENTIFIER ); // recurse at strictly higher precedence
    }
                                     
    return RenderPureIdentifier( id, surround_prod );
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderIntegral( TreePtr<Integral> type, Syntax::Production surround_prod ) try
{
	(void)surround_prod;
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
		Render::Dispatch( type, surround_prod );
		
	s += SSPrintf("/* %d bits */", width );

    return s;
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderIntegralTypeAndDeclarator( TreePtr<Integral> type, string declarator ) try
{
    // This function only exists to provide bitfields in member declarations that use declarators.
    // RenderIntegral() can provide the pure types directly, without bitfields.
    ASSERT(!declarator.empty())("I don't think bitfields can be used in anonymous types");

    int64_t width;
    auto ic = DynamicTreePtrCast<SpecificInteger>( type->width );
    ASSERT(ic)("width must be integer");
    width = ic->GetInt64();

    TRACE("width %" PRId64 "\n", width);

    string s = kit.render( type, Syntax::Production::SPACE_SEP_DECLARATION );

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


string CppRender::RenderFloating( TreePtr<Floating> type, Syntax::Production surround_prod ) try
{
	(void)surround_prod;
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
        Render::Dispatch( type, surround_prod );

    return s;
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderTypeAndDeclarator( TreePtr<Type> type, string declarator, 
                                           Syntax::Production declarator_prod, Syntax::Production surround_prod, bool constant ) try
{
	ASSERT( declarator != "🞊Node⦑⦒" );
    string const_str = constant?"const ":"";
    bool pure_type = (declarator == "");
    string sdeclarator;
    if( !pure_type )
        sdeclarator = " " + declarator;
    Syntax::Production type_prod = pure_type ? surround_prod  
                                             : Syntax::Production::SPACE_SEP_DECLARATION;
	if( !type )
        return const_str + kit.render( type, type_prod ) + sdeclarator;

    // Production passed in here comes from the current value of the delcarator string, not surrounding production.
    Syntax::Production prod_surrounding_declarator = type->GetOperandInDeclaratorProduction();
    ASSERT( prod_surrounding_declarator != Syntax::Production::UNDEFINED )
          ("Rendering type: ")(type)(" in production %d",(int)prod_surrounding_declarator)(" got no surrounding production\n");
    ASSERT( Syntax::GetPrecedence(prod_surrounding_declarator) <= Syntax::GetPrecedence(Syntax::Production::PARENTHESISED) ); // Can't satisfy this production's precedence demand using parentheses
    ASSERT( Syntax::GetPrecedence(declarator_prod) >= Syntax::GetPrecedence(Syntax::Production::BOOT_EXPR) ); // Can't put this node into parentheses
    bool parenthesise = Syntax::GetPrecedence(declarator_prod) < Syntax::GetPrecedence(prod_surrounding_declarator);  
    // Apply to object rather than recursing, because this is declarator    
    if( parenthesise )
        declarator = "(" + declarator + ")";
                
    TRACE();
    if( TreePtr<Integral> i = DynamicTreePtrCast< Integral >(type) )
        return const_str + RenderIntegralTypeAndDeclarator( i, declarator );        
    else if( TreePtr<Constructor> c = DynamicTreePtrCast< Constructor >(type) )
        return declarator + "(" + RenderParams(c) + ")" + const_str;
    else if( TreePtr<Destructor> f = DynamicTreePtrCast< Destructor >(type) )
        return declarator + "()" + const_str;
    else if( TreePtr<Function> f = DynamicTreePtrCast< Function >(type) )
        return RenderTypeAndDeclarator( f->return_type, declarator + "(" + RenderParams(f) + ")" + const_str, 
                                        Syntax::Production::POSTFIX, surround_prod );
    else if( TreePtr<Pointer> p = DynamicTreePtrCast< Pointer >(type) )
        return RenderTypeAndDeclarator( p->destination, string(DynamicTreePtrCast<Const>(p->constancy)?"const ":"") + "*" + const_str + declarator, 
                                        Syntax::Production::PREFIX, surround_prod, false ); // TODO Pointer node to indicate constancy of pointed-to object - would go into this call to RenderTypeAndDeclarator
    else if( TreePtr<Reference> r = DynamicTreePtrCast< Reference >(type) )
        return RenderTypeAndDeclarator( r->destination, string(DynamicTreePtrCast<Const>(p->constancy)?"const ":"") + "&" + const_str + declarator, 
                                        Syntax::Production::PREFIX, surround_prod );
    else if( TreePtr<Array> a = DynamicTreePtrCast< Array >(type) )
        return RenderTypeAndDeclarator( 
                           a->element, 
                           declarator + "[" + kit.render( a->size, Syntax::Production::BOOT_EXPR) + "]", 
                           Syntax::Production::POSTFIX,
                           surround_prod,
                           constant );
    else if( TreePtr<SpecificTypeIdentifier> ti = DynamicTreePtrCast< SpecificTypeIdentifier >(type) )
        return const_str + kit.render( ti, type_prod) + sdeclarator;
    else if( dynamic_pointer_cast<Labeley>(type) )
        return const_str + "void *" + declarator;
    else // Assume the type renders expressionally
        return const_str + kit.render( type, type_prod ) + sdeclarator;
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderType( TreePtr<CPPTree::Type> type, Syntax::Production surround_prod )
{
	if( DynamicTreePtrCast< Void >(type) )
        return "void";
    else if( DynamicTreePtrCast< Boolean >(type) )
        return "bool";
    	
    // If we got here, we should not be looking at a type that renders expressionally
	if( Syntax::GetPrecedence(type->GetMyProduction()) < Syntax::GetPrecedence(Syntax::Production::BOOT_EXPR) ) 
	{
		// Production ANONYMOUS relates to the fact that we've provided an empty string for the initial declarator.
		return RenderTypeAndDeclarator( type, "", Syntax::Production::ANONYMOUS, surround_prod, false ); 
	}
	else
		return Render::Dispatch( type, surround_prod );
}

// Insert escapes into a string so it can be put in source code
// TODO use \n \r etc and let printable ascii through
string CppRender::Sanitise( string s ) try
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


string CppRender::RenderOperator( TreePtr<Operator> op, Syntax::Production surround_prod ) try
{
    (void)surround_prod;
    ASSERT(op);
    
    string s;
    Sequence<Expression> operands;
    if( auto n = DynamicTreePtrCast< New >(op) )
        return string (DynamicTreePtrCast<Global>(n->global) ? "::" : "") +
               "new(" + RenderOperandSequence( n->placement_arguments ) + ") " +
               kit.render( n->type, Syntax::Production::TYPE_IN_NEW ) +
               (n->constructor_arguments.empty() ? "" : "(" + RenderOperandSequence( n->constructor_arguments ) + ")" );
    else if( auto d = DynamicTreePtrCast< Delete >(op) )
        return string(DynamicTreePtrCast<Global>(d->global) ? "::" : "") +
               "delete" +
               (DynamicTreePtrCast<DeleteArray>(d->array) ? "[]" : "") +
               " " + kit.render( d->pointer, Syntax::Production::PREFIX );
    else if( auto lu = DynamicTreePtrCast< Lookup >(op) )
        return kit.render( lu->object, Syntax::Production::POSTFIX ) + "." +
               kit.render( lu->member, Syntax::BoostPrecedence(Syntax::Production::POSTFIX) );
    else if( auto c = DynamicTreePtrCast< Cast >(op) )
        return "(" + kit.render( c->type, Syntax::Production::BOOT_EXPR ) + ")" +
               kit.render( c->operand, Syntax::Production::PREFIX );
    else if( auto condo = DynamicTreePtrCast< ConditionalOperator >(op) )
    {
        return kit.render( condo->condition, Syntax::BoostPrecedence(Syntax::Production::ASSIGN) ) + 
               " ? " +
               // Middle expression boots parser - so you can't split it up using (), [] etc
               kit.render( condo->expr_then, Syntax::Production::BOOT_EXPR ) + 
               " : " +
               kit.render( condo->expr_else, Syntax::Production::ASSIGN );          
    }
    else if( auto subs = DynamicTreePtrCast< Subscript >(op) )
    {
        return kit.render( subs->destination, Syntax::Production::POSTFIX ) + 
               "[" +
			   kit.render( subs->index, Syntax::Production::BOOT_EXPR ) + 
			   "]";
    }
    else if( auto al = DynamicTreePtrCast< ArrayLiteral >(op) )    
        return RenderOperandSequence( al->operands );
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
        s = kit.render( *operands_it, prod_left ); \
        s += TEXT; \
        ++operands_it; \
        s += kit.render( *operands_it, prod_right ); \
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
                paren = !ScopeResolvingPrefix( id, Syntax::Production::PROD ).empty(); \
        s += (paren?"(":"") + kit.render( *operands_it, Syntax::Production::PROD) + (paren?")":""); \
    }
#define POSTFIX(TOK, TEXT, NODE_SHAPED, BASE, CAT, PROD, ASSOC) \
    else if( DynamicTreePtrCast<NODE_SHAPED>(op) ) \
    { \
		Sequence<Expression>::iterator operands_it = operands.begin(); \
        s = kit.render( *operands_it, Syntax::Production::PROD ); \
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


string CppRender::RenderMapArgs( TreePtr<Type> dest_type, Collection<IdValuePair> &args ) try
{   
	list<string> ls;
	if( dest_type )
    {
		// Convert f->params from Parameters to Declarations
		Sequence<Declaration> param_sequence;   
		if( auto f = TreePtr<CallableParams>::DynamicCast(dest_type) )  
			for( auto param : f->params )
				param_sequence.push_back(param); 

		// Determine args sequence using param sequence
		Sequence<Expression> arg_sequence  = SortMapById( args, param_sequence );

		// Render to strings
		for( TreePtr<Expression> e : arg_sequence )
			ls.push_back( kit.render( e, Syntax::Production::COMMA_SEP ) );
    }
    else
    {
		// No type, so render map-style
        for( TreePtr<IdValuePair> mi : args )
			ls.push_back( kit.render( mi, Syntax::Production::COMMA_SEP ) );
	}    

    // Do the syntax
    return Join( ls, ", ", "(", ")" );
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderCall( TreePtr<Call> call, Syntax::Production surround_prod ) try
{
	(void)surround_prod;
    string s;

    // Render the expression that resolves to the function name unless this is
    // a constructor call in which case just the name of the thing being constructed.
    if( TreePtr<Expression> base = TypeOf::instance.TryGetConstructedExpression( kit, call ).GetTreePtr() )
        s += kit.render( base, Syntax::Production::POSTFIX );
    else
        s += kit.render( call->callee, Syntax::Production::POSTFIX );

    s += RenderMapArgs(TypeOf::instance.Get(kit, call->callee).GetTreePtr(), call->args);
    return s;
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderExprSeq( Sequence<Expression> seq ) try
{
    list<string> renders;
    for( TreePtr<Expression> e : seq )    
        renders.push_back( kit.render( e, Syntax::Production::COMMA_SEP) );               
    return Join(renders, ", ", "(", ")");
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderExteriorCall( TreePtr<SeqArgsCall> call, Syntax::Production surround_prod ) try
{
	(void)surround_prod;
    string args_in_parens = RenderExprSeq(call->arguments);

    // Constructor case: spot by use of Lookup to empty-named method. Elide the "."
    if( auto lu = DynamicTreePtrCast< Lookup >(call->callee) )
        if( auto id = DynamicTreePtrCast< InstanceIdentifier >(lu->member) )
            if( id->GetRenderTerminal().empty() )
                return kit.render( lu->object, Syntax::Production::POSTFIX ) + args_in_parens;

    // Other funcitons just evaluate
    return kit.render( call->callee, Syntax::Production::POSTFIX ) + args_in_parens;
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderMacroStatement( TreePtr<MacroStatement> ms, Syntax::Production surround_prod ) try
{
	(void)surround_prod;
	string s = kit.render( ms->identifier, Syntax::Production::POSTFIX );
	
    list<string> renders; // TODO duplicated code, factor out into RenderSeqMacroArgs()
    for( TreePtr<Node> node : ms->arguments )
        renders.push_back( kit.render( node, Syntax::Production::COMMA_SEP) );
    s += Join(renders, ", ", "(", ");\n");
    return s;
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderExpression( TreePtr<Initialiser> expression, Syntax::Production surround_prod ) try
{
    (void)surround_prod;
      
    if( auto ce = DynamicTreePtrCast< StatementExpression >(expression) )
    {
        string s = "({ ";
        AutoPush< TreePtr<Node> > cs( scope_stack, ce );
        s += RenderDeclScope( ce ); // Must do this first to populate backing list
        for( TreePtr<Statement> st : ce->statements )    
            s += kit.render( st, Syntax::Production::STATEMENT_LOW );    
        return s + " })";
    }
    else if( auto pot = DynamicTreePtrCast< SizeOf >(expression) )
        return "sizeof(" + kit.render( pot->argument, Syntax::Production::BOOT_EXPR ) + ")";               
    else if( auto pot = DynamicTreePtrCast< AlignOf >(expression) )
        return "alignof(" + kit.render( pot->argument, Syntax::Production::BOOT_EXPR ) + ")";    
    else
        return Render::Dispatch( expression, surround_prod );

}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderMakeRecord( TreePtr<RecordLiteral> make_rec, Syntax::Production surround_prod ) try
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
    // be applied to the Collection<IdValuePair> in order to get a match-up between sub-expressions
    // and fields. I think C++ side-steps this by diallowing the RecordLiteral syntax
    // in classes where dependencies might matter.

    TreePtr<Record> r = TryGetRecordDeclaration(kit, id).GetTreePtr();
    // Make sure we have the same ordering as when the record was rendered
    Sequence<Declaration> sorted_members = SortDecls( r->members, true, unique_names );

    // Determine args sequence using param sequence
    Sequence<Expression> sub_expr_sequence  = SortMapById( make_rec->operands, sorted_members );
    
    // Render to strings
    list<string> ls;
    for( TreePtr<Expression> e : sub_expr_sequence )
        ls.push_back( kit.render( e, Syntax::Production::COMMA_SEP ) );

    // Do the syntax
    s += "(" + kit.render( make_rec->type, Syntax::Production::BOOT_EXPR ) + ")"; 
    s += Join( ls, ", ", "{", "}" );   // Use of {} in expressions is irregular so handle locally 
    return s;
}
DEFAULT_CATCH_CLAUSE


Sequence<Expression> CppRender::SortMapById( Collection<IdValuePair> &id_value_map,
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
                for( TreePtr<IdValuePair> mi : id_value_map )
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


string CppRender::RenderAccessSpec( TreePtr<AccessSpec> access, Syntax::Production surround_prod ) try
{
	(void)surround_prod;
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


string CppRender::RenderStorage( TreePtr<Instance> st ) try
{
    if( !scope_stack.empty() && DynamicTreePtrCast<Program>( scope_stack.top() ) )
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
        return ""; // eg for Instance as a wildcard
}
DEFAULT_CATCH_CLAUSE


void CppRender::ExtractInits( Sequence<Statement> &body, 
                              Sequence<Statement> &inits, 
                              Sequence<Statement> &remainder )
{
	(void)kit;
    // Initialisers are just calls to the constructor embedded in the body. In Inferno,
    // we call a constructor by 
    for( TreePtr<Statement> s : body )
    {
        if( auto call = DynamicTreePtrCast< GoSub >(s) ) // TODO drop after changeover to Construction
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


string CppRender::RenderInstanceProto( TreePtr<Instance> o ) try
{
    string s;
    bool constant=false;

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
		name = ScopeResolvingPrefix(o->identifier, Syntax::Production::SCOPE_RESOLVE);
        // TODO use TryGetRecordDeclaration( Typeof( o->identifier ) ) and leave scopes out of it
        TreePtr<Record> rec = DynamicTreePtrCast<Record>( TryGetScope( o->identifier ) );
        ASSERT( rec );        
        name += (de ? "~" : ""); 
        starting_declarator_prod = Syntax::Production::PURE_IDENTIFIER; // we already rendered the scope prefix into name
        name += kit.render( rec->identifier, starting_declarator_prod);
    }
    else
    {
        starting_declarator_prod = Syntax::Production::SCOPE_RESOLVE;
        name += kit.render( o->identifier, starting_declarator_prod);
    }

    s += RenderTypeAndDeclarator( o->type, name, starting_declarator_prod, Syntax::Production::PROTOTYPE, constant );

    return s;
} 
DEFAULT_CATCH_CLAUSE  


string CppRender::RenderInitialisation( TreePtr<Initialiser> init ) try
{
	string s;
	if( TreePtr<Expression> ei = DynamicTreePtrCast<Expression>( init ) )
    {
        // Attempt direct initialisation by providing args for a constructor call
        if( auto call = DynamicTreePtrCast<SeqArgsCall>( ei ) )
        {
            if( auto lu = TreePtr<Lookup>::DynamicCast(call->callee) )
                if( auto id = TreePtr<InstanceIdentifier>::DynamicCast(lu->member) )
                    if( id->GetRenderTerminal().empty() ) // syscall to a nameless member function => sys construct
                        return RenderExprSeq(call->arguments) + ";\n";
        }
        if( auto call = DynamicTreePtrCast<Call>( ei ) ) try
        {       
            if( TypeOf::instance.TryGetConstructedExpression( kit, call ).GetTreePtr() )        
                return RenderMapArgs(TypeOf::instance.Get(kit, call->callee).GetTreePtr(), call->args) + ";\n";
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
        ExtractInits( code, inits, remainder );

        // Render the constructor initialisers if there are any
        if( !inits.empty() )
        {
            s += " :\n";
            s += RenderConstructorInitList( inits );

			// Render the other stuff as a Compound so we always get {} in all cases
			auto r = MakeTreeNode<Compound>();
			r->members = members;
			r->statements = remainder;		
			init = r;	
		}	
    }

    return s + kit.render( init, Syntax::Production::INITIALISER); 
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderInstance( TreePtr<Instance> o, Syntax::Production surround_prod )
{
    string s;
       
	switch( surround_prod )
	{
		case Syntax::Production::DEFINITION:
			{
				// Definition is out-of-line so skip the storage
				s += RenderInstanceProto( o );
				AutoPush< TreePtr<Node> > cs( scope_stack, TryGetScope( o->identifier ) );
				s += RenderInitialisation( o->initialiser );	
				s += "\n";		
			}
			break;
			
		default: // should be one of the statement-level prods
			s += RenderStorage( o );
			s += RenderInstanceProto( o );
		    if( ShouldSplitInstance(o) )
			{
				// Emit just a prototype now and request definition later
				s += ";\n";
				// Split out the definition of the instance for rendering later at Program scope
				definitions.push(o);
			}		
			else
			{
				// Emit the whole lot in-line
				AutoPush< TreePtr<Node> > cs( scope_stack, TryGetScope( o->identifier ) );
				s += RenderInitialisation( o->initialiser );							
				s += "\n";		
			}
	}
	return s;
}


// Non-const static objects in records and functions 
// get split into a part that goes into the record (main line of rendering) and
// a part that goes separately (definitions gets appended at the very end).
// Do all functions, since SortDecls() ignores function bodies for dep analysis
bool CppRender::ShouldSplitInstance( TreePtr<Instance> o ) 
{
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
        if( scope_stack.empty() )
			return false;
			
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


string CppRender::RenderMacroDeclaration( TreePtr<MacroDeclaration> md, Syntax::Production surround_prod )
{
	(void)surround_prod;	
    // ---- Proto ----
	string s = kit.render( md->identifier, Syntax::Production::POSTFIX );
	list<string> renders;
	for( TreePtr<Node> node : md->arguments )
		renders.push_back( kit.render( node, Syntax::Production::COMMA_SEP) );
	s += Join(renders, ", ", "(", ")");
	
	// ---- Initialisation ----	    
    return s + RenderInitialisation( md->initialiser );
}


string CppRender::RenderRecordProto( TreePtr<Record> record )
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
        s += Render::Dispatch( record, Syntax::Production::SPACE_SEP_DECLARATION );

    // Name of the record
    s += " " + kit.render( record->identifier, Syntax::Production::SPACE_SEP_DECLARATION);
    
    return s;
}


string CppRender::RenderPreProcDecl( TreePtr<PreProcDecl> ppd, Syntax::Production surround_prod ) try
{
	(void)surround_prod;
    if( auto si = TreePtr<SystemInclude>::DynamicCast(ppd) )
        return "#include <" + si->filename->GetString() + ">";
    else if( auto si = TreePtr<LocalInclude>::DynamicCast(ppd) )
        return "#include " + si->filename->GetRenderTerminal();
    else
        return ERROR_UNSUPPORTED(ppd);     
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderDeclaration( TreePtr<Declaration> declaration, Syntax::Production surround_prod ) try
{
    TRACE();
    string s;
	(void)surround_prod;

    if( TreePtr<Typedef> t = DynamicTreePtrCast< Typedef >(declaration) )
    {
        Syntax::Production starting_declarator_prod = Syntax::Production::PURE_IDENTIFIER;
        auto id = kit.render( t->identifier, starting_declarator_prod);
        s += "typedef " + RenderTypeAndDeclarator( t->type, id, starting_declarator_prod, Syntax::Production::SPACE_SEP_DECLARATION );
    }
    else if( TreePtr<Record> r = DynamicTreePtrCast< Record >(declaration) )
    {
        // Prototype of the record
        s += RenderRecordProto( r );
        
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
                    s += kit.render( b->access, Syntax::Production::TOKEN ) + " ";
                    s += kit.render( b->record, Syntax::Production::SCOPE_RESOLVE);
                }
            }
        }

        // Members
        s += "\n{ // memb\n";
        TreePtr<AccessSpec> a = r->GetInitialAccess();
        AutoPush< TreePtr<Node> > cs( scope_stack, r );
        if( a )
            s += RenderDeclScope( r, a );          
        else
            s += RenderEnumBodyScope( r ); 
            
        s += "};\n";
        // Add blank lines before and after
        s = '\n' + s + '\n';
    }
    else if( TreePtr<Label> l = DynamicTreePtrCast<Label>(declaration) )
        return kit.render( l->identifier, Syntax::Production::PURE_IDENTIFIER) + ":;\n"; // need ; after a label in case last in compound block
    else
        s += Render::Dispatch( declaration, surround_prod );

    TRACE();
    return s;
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderStatement( TreePtr<Statement> statement, Syntax::Production surround_prod ) try
{
    (void)surround_prod;
    TRACE();
    ASSERT( statement );
    //printf( "%s %d things\n", typeid(*statement).name(), statement->Itemise().size() );
    if( TreePtr<Declaration> d = DynamicTreePtrCast< Declaration >(statement) )
        return RenderDeclaration( d, surround_prod );
    else if( TreePtr<Compound> c = DynamicTreePtrCast< Compound >(statement) )
    {
        string s;
        AutoPush< TreePtr<Node> > cs( scope_stack, c );
        s += RenderDeclScope( c ); // Must do this first to populate backing list
        for( TreePtr<Statement> st : c->statements )    
            s += kit.render( st, Syntax::Production::STATEMENT_LOW );    
        return s;
    }
    else if( TreePtr<Expression> e = DynamicTreePtrCast< Expression >(statement) )
        return kit.render( e, surround_prod);
    else if( TreePtr<Return> es = DynamicTreePtrCast<Return>(statement) )
        return "return " + kit.render( es->return_value, Syntax::Production::SPACE_SEP_STATEMENT);
    else if( TreePtr<Goto> g = DynamicTreePtrCast<Goto>(statement) )
    {
        if( TreePtr<SpecificLabelIdentifier> li = DynamicTreePtrCast< SpecificLabelIdentifier >(g->destination) )
            return "goto " + kit.render( li, Syntax::Production::SPACE_SEP_STATEMENT);  // regular goto
        else
            return "goto *" + kit.render( g->destination, Syntax::Production::PREFIX); // goto-a-variable (GCC extension)
    }
    else if( TreePtr<If> i = DynamicTreePtrCast<If>(statement) )
    {
        bool has_else_clause = !DynamicTreePtrCast<Nop>(i->body_else); // Nop means no else clause
        string s;
        s += "if( " + kit.render( i->condition, Syntax::Production::CONDITION) + " )\n";
        // The choice of production here causes then "else" ambuguity to be resolved.
        s += kit.render( i->body, has_else_clause ? Syntax::Production::STATEMENT_HIGH : Syntax::Production::STATEMENT_LOW);
        if( has_else_clause )  
            s += "else\n" + kit.render( i->body_else, Syntax::Production::STATEMENT_LOW);
        return s;
    }
    else if( TreePtr<While> w = DynamicTreePtrCast<While>(statement) )
        return "while( " + 
               kit.render( w->condition, Syntax::Production::CONDITION) + " )\n" +
               kit.render( w->body, surround_prod);
    else if( TreePtr<Do> d = DynamicTreePtrCast<Do>(statement) )
        return "do\n" +
               kit.render( d->body, Syntax::Production::STATEMENT_LOW) +
               "while( " + kit.render( d->condition, Syntax::Production::CONDITION) + " )";
    else if( TreePtr<For> f = DynamicTreePtrCast<For>(statement) )
        return "for( " + 
               kit.render( f->initialisation, Syntax::Production::STATEMENT_LOW) + 
               kit.render( f->condition, Syntax::Production::STATEMENT_LOW) + 
               kit.render( f->increment, Syntax::Production::BOOT_EXPR) + " )\n" +
               kit.render( f->body, surround_prod);
    else if( TreePtr<Switch> s = DynamicTreePtrCast<Switch>(statement) )
        return "switch( " + kit.render( s->condition, Syntax::Production::CONDITION) + " )\n" +
               kit.render( s->body, surround_prod);
    else if( TreePtr<Case> c = DynamicTreePtrCast<Case>(statement) )
        return "case " + kit.render( c->value, Syntax::Production::SPACE_SEP_STATEMENT) + ":";
    else if( TreePtr<RangeCase> rc = DynamicTreePtrCast<RangeCase>(statement) )
        // GCC extension: assume that ... is part of the case statement, and can boot the expressions.
        return "case " + 
               kit.render( rc->value_lo, Syntax::Production::SPACE_SEP_STATEMENT) + 
               " ... " + 
               kit.render( rc->value_hi, Syntax::Production::SPACE_SEP_STATEMENT) + 
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
        return Render::Dispatch( statement, surround_prod );
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderConstructorInitList( Sequence<Statement> spe ) try
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
            s += kit.render( e, Syntax::Production::COMMA_SEP );
        else 
            s += ERROR_UNSUPPORTED(st);
        first = false;
    }
    return s;
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderEnumBodyScope( TreePtr<CPPTree::Record> record ) try
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
        s += kit.render( o->identifier, Syntax::BoostPrecedence(Syntax::Production::ASSIGN)) + " = ";
        
        auto ei = TreePtr<Expression>::DynamicCast( o->initialiser );
        if( !ei )
        {
            s += ERROR_UNSUPPORTED(o->initialiser);
            continue;
        }       
        s += kit.render( ei, Syntax::Production::ASSIGN);

        first = false;    
    }
    return s + "\n";
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderOperandSequence( Sequence<Expression> spe ) try
{
    TRACE();
    string s;
	list<string> renders;    
    for( TreePtr<Expression> pe : spe )
		renders.push_back( kit.render( pe, Syntax::Production::COMMA_SEP ) );
    return Join(renders, ", ", "{", "}"); // Use of {} in expressions is irregular so handle locally 
}
DEFAULT_CATCH_CLAUSE


string CppRender::MaybeRenderFieldAccess( TreePtr<Declaration> declaration,
                                       TreePtr<AccessSpec> *current_access )
{
    ASSERT( current_access );
    
    TreePtr<AccessSpec> this_access = MakeTreeNode<Public>();

    // Decide access spec for this declaration (explicit if instance, otherwise force to Public)
    if( TreePtr<Field> f = DynamicTreePtrCast<Field>(declaration) )
        this_access = f->access;

    return MaybeRenderAccessColon( this_access, current_access );  
}                                 


string CppRender::MaybeRenderAccessColon( TreePtr<AccessSpec> this_access,
                                       TreePtr<AccessSpec> *current_access )
{
    ASSERT( current_access );

    // Now decide whether we actually need to render an access spec (ie has it changed?)
    if( typeid(*this_access) != typeid(**current_access) ) // current_access spec must have changed
    {
        *current_access = this_access;
        return kit.render( this_access, Syntax::Production::TOKEN ) + ":\n";
    }
    
    return "";  
}                                 


string CppRender::RenderDeclScope( TreePtr<DeclScope> decl_scope,
                                   TreePtr<AccessSpec> init_access ) try
{
    TRACE();
    Sequence<Declaration> sorted = SortDecls( decl_scope->members, true, unique_names );

    // Emit a prototype for each record and preproc
    string s;
    for( TreePtr<Declaration> pd : sorted ) //for( int i=0; i<sorted.size(); i++ )
    {       
        if( auto ppd = DynamicTreePtrCast<PreProcDecl>(pd) )
        {
            s += kit.render( ppd, Syntax::Production::DECLARATION ) + "\n";
            continue;
        }
        
        TreePtr<Record> r = DynamicTreePtrCast<Record>(pd); 
        if( !r )
            continue; // only do records and preprocessor decls
            
        if( DynamicTreePtrCast<Enum>(r) ) 
            continue; // but not an enum
    
        if( init_access )
            s += MaybeRenderFieldAccess( r, &init_access );
        //s += RenderRecordProto( r ) + "; // RDS-record proto\n";   
        s += kit.render( r, Syntax::Production::PROTOTYPE ) + "; // RDS-record proto (new)\n"; 
    }
    
    // Emit the actual definitions, sorted for dependencies
    for( TreePtr<Declaration> d : sorted )
    {
        if( DynamicTreePtrCast<PreProcDecl>(d) )
            continue;
            
        if( init_access )
            s += MaybeRenderFieldAccess( d, &init_access );        
        s += kit.render( d, Syntax::Production::STATEMENT_LOW );
    }
    TRACE();
    return s;
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderParams( TreePtr<CallableParams> key ) try
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
        string name = kit.render( o->identifier, starting_declarator_prod);
        s += RenderTypeAndDeclarator( o->type, name, starting_declarator_prod, Syntax::Production::BARE_DECLARATION, false );
            
        first = false;
    }
    return s;
}
DEFAULT_CATCH_CLAUSE
