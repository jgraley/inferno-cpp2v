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
#include "vn/agents/agent.hpp"
#include "clang/Parse/DeclSpec.h"
#include "vn/lang/uniquify_identifiers.hpp"
#include "cpprender.hpp"

using namespace std;
using namespace CPPTree;

// TODO indent back to previous level at end of string
#define ERROR_UNKNOWN(V) \
    ( string( "\n❌" ) + \
      string( V ) + \
      string( " not supported in " ) + \
      string( __func__ ) + \
      string( "()❌\n" ) )

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
	Render( GetDefaultPolicy(), of ) // default policy when rendering to pure C++
{
}


string CppRender::RenderToString( TreePtr<Node> root )
{       
    // Context is used for various lookups but does not need
    // to be a Scope.
    context = root; 
        
    utils = make_unique<DefaultTransUtils>(context);
    using namespace placeholders;

    // Make the identifiers unique (does its own tree walk). Be strict about undeclared
    // identifiers - to rename them would be unsafe because we assume there's
    // a declaration ouside of our tree. This actually gets other nodes too but 
    // we'll only look up identifiers.
    UniquifyNames::Policy un_policy {
	    .name_getter = &Syntax::GetIdentifierName, 
		.include_single_parent = true,
		.include_multi_parent = true,
		.include_designation_named_identifiers = false,
		.preserve_undeclared_ids = true
	};
	UniquifyNames identifiers_uniqifier(un_policy); 
    trans_kit = TransKit{ utils.get() };
    unique_identifier_names = identifiers_uniqifier.UniquifyAll( trans_kit, context );
    
    Syntax::Policy top_policy = default_policy;
    return DoRender( root, Syntax::Production::PROGRAM, top_policy );
}	


Syntax::Policy CppRender::GetDefaultPolicy()
{
	Syntax::Policy policy;
	
	// Insert braces to disambiguate stratements eg in case of if/else ambiguity
	policy.boot_statements_using_braces = true;
	
	return policy;
}


Syntax::Production CppRender::GetNodeProduction( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy ) const
{
	(void)surround_prod;
	try
	{
		return node->GetMyProduction(this, policy);       
	}
	catch( Syntax::Refusal &r )
	{
		ASSERT(false)(node)(" unknown production");
	}
	ASSERTFAIL();
}


string CppRender::Dispatch( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy ) try 
{ 		
	return node->GetRender( this, surround_prod, policy );		
}
catch( Syntax::Refusal & ) 
{	
	return DispatchInternal( node, surround_prod, policy );
}


string CppRender::DispatchInternal( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy )
{			
    if( TreePtr<Uninitialised>::DynamicCast(node) )
        return string();  
    else if( auto program = TreePtr<Program>::DynamicCast(node) )
        return RenderProgram( program, surround_prod );
    else if( auto access = TreePtr<AccessSpec>::DynamicCast(node) ) // Identifier can be a kind of type or expression
        return RenderAccessSpec( access, surround_prod, policy );
    else if( auto floating = TreePtr<Floating>::DynamicCast(node) )
        return RenderFloating( floating, surround_prod, policy );
    else if( auto integral = TreePtr<Integral>::DynamicCast(node) )
        return RenderIntegral( integral, surround_prod, policy );
    else if( auto type = TreePtr<Type>::DynamicCast(node) )  // Type is a kind of Operator
        return RenderType( type, surround_prod, policy );
    else if( auto literal = DynamicTreePtrCast< Literal >(node) )
        return RenderLiteral( literal, surround_prod );
    else if( auto call = TreePtr<Call>::DynamicCast(node) )
        return RenderMapArgsCallAsSeqArg( call, surround_prod ); // Still need this for map args resolution TODO raise/lower steps
    else if( auto make_rec = TreePtr<RecordLiteral>::DynamicCast(node) )
        return RenderMakeRecord( make_rec, surround_prod );
    else if( auto macro_decl = TreePtr<MacroDeclaration>::DynamicCast(node) )
        return RenderMacroDeclaration( macro_decl, surround_prod );
    else if( auto macro_stmt = TreePtr<MacroStatement>::DynamicCast(node) )
        return RenderMacroStatement( macro_stmt, surround_prod );
//    else if( auto op = TreePtr<Operator>::DynamicCast(node) ) // Operator is a kind of Expression
//        return RenderOperator( op, surround_prod );
    else if( auto expression = TreePtr<Expression>::DynamicCast(node) ) // Expression is a kind of Statement
        return RenderExpression( expression, surround_prod, policy );
    else if( auto instance = TreePtr<Instance>::DynamicCast(node) )    // Instance is a kind of Statement and Declaration
        return RenderInstance( instance, surround_prod, policy ); 
    else if( auto ppd = TreePtr<PreProcDecl>::DynamicCast(node) )
        return RenderPreProcDecl(ppd, surround_prod); 
    else if( auto declaration = TreePtr<Declaration>::DynamicCast(node) )
        return RenderDeclaration( declaration, surround_prod, policy );
    else if( auto statement = TreePtr<Statement>::DynamicCast(node) )
        return RenderStatement( statement, surround_prod, policy );
        
    // Due #969 we might have a standard agent, so fall back to a function that
    // definitely won't call any agent methods.
    return RenderNodeExplicit( node, surround_prod, policy );      
}


string CppRender::RenderProgram( TreePtr<CPPTree::Program> program, Syntax::Production surround_prod )
{
	(void)surround_prod;
    string s;

    // Track scopes for name resolution
    s += RenderDeclScope( program ); // gets the .hpp stuff directly 
    
    s += "\n// Definitions";    
    
    // These are rendered here, inside program scope but outside any additional scopes
    // that were on the scope stack when the instance was seen. These could go in a .cpp file.
    while( !definitions.empty() )
    {
		Syntax::Policy definition_policy = default_policy;
		definition_policy.force_initialisation = true;
        s += "\n";
        s += DoRender( definitions.front(), Syntax::Production::DECLARATION, definition_policy ); 
        s += "\n";
        definitions.pop();
    }
    return s;  
}


string CppRender::RenderLiteral( TreePtr<Literal> sp, Syntax::Production surround_prod ) try
{
	(void)surround_prod;
    return Sanitise( sp->GetRender(this, surround_prod, default_policy) );
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderScopeResolvingPrefix( TreePtr<Node> node ) try
{
    TreePtr<Node> scope = TryGetScope(node);
              
    if( !scope )
        return ""; // either we're not in a scope or id is undeclared
    else if( DynamicTreePtrCast<Program>( scope ) )
        return "";
    else if( auto e = DynamicTreePtrCast<Enum>( scope ) ) // <- for enum
        return RenderScopeResolvingPrefix( e->identifier );    // omit scope for the enum itself
    else if( auto r = DynamicTreePtrCast<Record>( scope ) ) // <- for class, struct, union
        return r->identifier->GetRender(this, Syntax::Production::RESOLVER, default_policy) + "::";
    else if( DynamicTreePtrCast<CallableParams>( scope ) ||  // <- this is for params
             DynamicTreePtrCast<Compound>( scope ) ||    // <- this is for locals in body
             DynamicTreePtrCast<StatementExpression>( scope ) )    // <- this is for locals in body
        return "";
    else
        return scope->GetTrace()+"::"; // unknown scope
}
DEFAULT_CATCH_CLAUSE


string CppRender::GetUniqueIdentifierName( TreePtr<Node> id ) const 
{
	ASSERT( unique_identifier_names.count(id) > 0 )
	      (id)
	      (" (\"%s\") missing from unique_identifier_names", id->GetIdentifierName().c_str() );
	return unique_identifier_names.at(id);
}


string CppRender::RenderIntegral( TreePtr<Integral> type, Syntax::Production surround_prod, Syntax::Policy policy ) try
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
		Render::Dispatch( type, surround_prod, policy );
		
	if( ReadArgs::use.count("c") )
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

    string s = DoRender( type, Syntax::Production::SPACE_SEP_DECLARATION );

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


string CppRender::RenderFloating( TreePtr<Floating> type, Syntax::Production surround_prod, Syntax::Policy policy ) try
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
        Render::Dispatch( type, surround_prod, policy );

    return s;
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderTypeAndDeclarator( TreePtr<Type> type, string declarator, 
                                           Syntax::Production declarator_prod, Syntax::Production surround_prod, bool constant ) try
{
	ASSERT( declarator != "🞊Node【】" );
    string const_str = constant?"const ":"";
    bool pure_type = (declarator == "");
    string sdeclarator;
    if( !pure_type )
        sdeclarator = " " + declarator;
    Syntax::Production type_prod = pure_type ? surround_prod  
                                             : Syntax::Production::SPACE_SEP_DECLARATION;
	if( !type )
        return const_str + DoRender( type, type_prod ) + sdeclarator;

    // Production passed in here comes from the current value of the delcarator string, not surrounding production.
    Syntax::Production prod_surrounding_declarator = type->GetOperandInDeclaratorProduction();
    ASSERT( Syntax::GetPrecedence(prod_surrounding_declarator) <= Syntax::GetPrecedence(Syntax::Production::BRACKETED) ); // Can't satisfy this production's precedence demand using parentheses
    ASSERT( Syntax::GetPrecedence(declarator_prod) >= Syntax::GetPrecedence(Syntax::Production::BOTTOM_EXPR) ); // Can't put this node into parentheses
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
                           declarator + "[" + DoRender( a->size, Syntax::Production::BOTTOM_EXPR) + "]", 
                           Syntax::Production::POSTFIX,
                           surround_prod,
                           constant );
    else if( TreePtr<SpecificTypeIdentifier> ti = DynamicTreePtrCast< SpecificTypeIdentifier >(type) )
        return const_str + DoRender( ti, type_prod) + sdeclarator;
    else if( dynamic_pointer_cast<Labeley>(type) )
        return "const void *" + declarator; // Always const
    else // Assume the type renders expressionally
        return const_str + DoRender( type, type_prod ) + sdeclarator;
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderType( TreePtr<CPPTree::Type> type, Syntax::Production surround_prod, Syntax::Policy policy )
{
	if( DynamicTreePtrCast< Void >(type) )
        return "void";
    else if( DynamicTreePtrCast< Boolean >(type) )
        return "bool";
    	
    // If we got here, we should not be looking at a type that renders expressionally
	if( Syntax::GetPrecedence(type->GetMyProductionTerminal()) < Syntax::GetPrecedence(Syntax::Production::BOTTOM_EXPR) ) 
	{
		// Production ANONYMOUS relates to the fact that we've provided an empty string for the initial declarator.
		return RenderTypeAndDeclarator( type, "", Syntax::Production::ANONYMOUS, surround_prod, false ); 
	}
	else
		return Render::Dispatch( type, surround_prod, policy );
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
    {
        ASSERTFAIL(); // Hold this spot for resolving the map args to the constructor
    }
    else
        return RenderNodeExplicit( op, surround_prod, default_policy );
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderMapArgs( TreePtr<Type> callee_type, TreePtr<MapArgumentation> map_argumentation ) try
{   
	list<string> ls;
	ASSERT( callee_type );
    
	// Convert f->params from Parameters to Declarations and settle on an arbitrary 
	// ordering. This needs to be the same on each visit with a given callee.
	Sequence<Declaration> decl_sequence;   
	if( auto f = TreePtr<CallableParams>::DynamicCast(callee_type) )  
		for( auto param : f->params )
			decl_sequence.push_back(param); 

	// Determine args sequence using param sequence
	TreePtr<SeqArgumentation> sa = MakeSeqArgumentation( map_argumentation, decl_sequence );

	// Let the SeqArgumentation node do the actual render
	return sa->DirectRenderArgumentation(this, default_policy);
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderMapArgsCallAsSeqArg( TreePtr<Call> call, Syntax::Production surround_prod ) try
{
	(void)surround_prod;
	// Note: we need to operate on the call, so that we can use callee to find the declaration and 
	// resolve the map into a sequence.

	auto map_args = TreePtr<MapArgumentation>::DynamicCast( call->argumentation );
	ASSERT( map_args );

    // Render the expression that resolves to the function name unless this is
    // a constructor call in which case just the name of the thing being constructed.
    string s = DoRender( call->callee, Syntax::Production::POSTFIX );

	// A map-args call isn't C++, so lower it to sequential args - requires the function type
    s += RenderMapArgs(TypeOf::instance.Get(trans_kit, call->callee).GetTreePtr(), map_args);
    return s;
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderMacroStatement( TreePtr<MacroStatement> ms, Syntax::Production surround_prod ) try
{
	(void)surround_prod;
	string s = DoRender( ms->identifier, Syntax::Production::POSTFIX );
	
    list<string> renders; // TODO duplicated code, factor out into RenderSeqMacroArgs()
    for( TreePtr<Node> node : ms->arguments )
        renders.push_back( DoRender( node, Syntax::Production::COMMA_SEP) );
    s += Join(renders, ", ", "(", ");\n");
    return s;
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderExpression( TreePtr<Initialiser> expression, Syntax::Production surround_prod, Syntax::Policy policy ) try
{
    (void)surround_prod;
      
    if( auto ce = DynamicTreePtrCast< StatementExpression >(expression) )
    {
        string s = "({ ";
        s += RenderDeclScope( ce ); // Must do this first to populate backing list
        for( TreePtr<Statement> st : ce->statements )    
            s += DoRender( st, Syntax::Production::STATEMENT_LOW );    
        return s + " })";
    }
    else if( auto pot = DynamicTreePtrCast< SizeOf >(expression) )
        return "sizeof(" + DoRender( pot->argument, Syntax::Production::BOTTOM_EXPR ) + ")";               
    else if( auto pot = DynamicTreePtrCast< AlignOf >(expression) )
        return "alignof(" + DoRender( pot->argument, Syntax::Production::BOTTOM_EXPR ) + ")";    
    else
        return Render::Dispatch( expression, surround_prod, policy );

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

    TreePtr<Record> r = TryGetRecordDeclaration(trans_kit, id).GetTreePtr();
    // Make sure we have the same ordering as when the record was rendered
    Sequence<Declaration> sorted_members = SortDecls( r->members, true, unique_identifier_names );

    // Determine args sequence using param sequence
    Sequence<Expression> sub_expr_sequence = SortMapById( make_rec->operands, sorted_members );
    
    // Render to strings
    list<string> ls;
    for( TreePtr<Expression> e : sub_expr_sequence )
        ls.push_back( DoRender( e, Syntax::Production::COMMA_SEP ) );

    // Do the syntax
    s += "(" + DoRender( make_rec->type, Syntax::Production::BOTTOM_EXPR ) + ")"; 
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


TreePtr<SeqArgumentation> CppRender::MakeSeqArgumentation( TreePtr<MapArgumentation> map_argumentation,
														   Sequence<Declaration> key_sequence )
{
	auto sa = MakeTreeNode<SeqArgumentation>();
	sa->arguments = SortMapById( map_argumentation->arguments, key_sequence ); // TODO could absorb
	return sa;
}											   


string CppRender::RenderAccessSpec( TreePtr<AccessSpec> access, Syntax::Production surround_prod, Syntax::Policy policy ) try
{
	if( type_index(typeid(*access)) == policy.current_access && 
	    surround_prod == Syntax::Production::BARE_DECLARATION )
		return "";
		
	(void)surround_prod;
	string s;
    if( DynamicTreePtrCast<Public>( access ) )
        s = "public";
    else if( DynamicTreePtrCast<Private>( access ) )
        s = "private";
    else if( DynamicTreePtrCast<Protected>( access ) )
        s = "protected";
    else
        s = ERROR_UNKNOWN("access spec");
    
    if( surround_prod == Syntax::Production::BARE_DECLARATION )
		s += ":\n";
	
	return s;
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderStorage( TreePtr<Instance> st, Syntax::Policy policy ) try
{
	// In VN, Static means in the C++ sense not the C sense
    if( DynamicTreePtrCast<Static>( st ) && policy.permit_static_keyword )
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
    // Initialisers are just calls to the constructor embedded in the body. In Inferno,
    // we call a constructor by 
    for( TreePtr<Statement> s : body )
    {
        if( auto call = DynamicTreePtrCast< MembInitialisation >(s) ) 
			inits.push_back(s);
		else
		    remainder.push_back(s);
    }
}


string CppRender::RenderInstanceProto( TreePtr<Instance> o, Syntax::Production starting_declarator_prod ) try
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
    TreePtr<Constructor> con = DynamicTreePtrCast<Constructor>(o->type);
    TreePtr<Destructor> de = DynamicTreePtrCast<Destructor>(o->type);
    if( con || de )
    {
		// Do the scope resolution separately so can insert ~
		if( starting_declarator_prod < Syntax::Production::PURE_IDENTIFIER )
			name = RenderScopeResolvingPrefix(o->identifier);
			
        // TODO use TryGetRecordDeclaration( Typeof( o->identifier ) ) and leave scopes out of it
        TreePtr<Record> rec = DynamicTreePtrCast<Record>( TryGetScope( o->identifier ) );
        ASSERT( rec );        
        name += (de ? "~" : ""); 
        starting_declarator_prod = Syntax::Production::PURE_IDENTIFIER; // we already rendered the scope prefix into name
        name += DoRender( rec->identifier, starting_declarator_prod);
    }
    else
    {
        //starting_declarator_prod = Syntax::Production::RESOLVER; // TODO only RESOLVER in definitions, otherwise PURE_IDENTIFIER
        name += DoRender( o->identifier, starting_declarator_prod);
    }

    s += RenderTypeAndDeclarator( o->type, name, starting_declarator_prod, Syntax::Production::BARE_DECLARATION, constant );

    return s;
} 
DEFAULT_CATCH_CLAUSE  


string CppRender::RenderInitialisation( TreePtr<Initialiser> init ) try
{
	string s;
	if( ReadArgs::use.count("c") )
		s += "/* RenderInitialisation(" + Trace(init) + ") */";
	if( auto stmt = DynamicTreePtrCast<Statement>(init) )
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
	
    return s + DoRender( init, Syntax::Production::INITIALISER); 
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderInstance( TreePtr<Instance> o, Syntax::Production surround_prod, Syntax::Policy policy )
{
    string s;
    
	if( policy.force_initialisation )
	{
		if( ReadArgs::use.count("c") )
			s += "/* force init */";   
		// Definition is out-of-line so skip the storage
		policy.force_initialisation = false; // stop at one level
		s += RenderInstanceProto( o, Syntax::Production::RESOLVER );
		s += RenderInitialisation( o->initialiser );	
		s += "\n";		
	}
	else 
	{		
		if( ReadArgs::use.count("c") )
			s += "/* no force init */";   
		s += RenderStorage( o, policy );
		s += RenderInstanceProto( o, Syntax::Production::PURE_IDENTIFIER );
		if( ShouldSplitInstance(o, surround_prod, policy) )
		{
			if( ReadArgs::use.count("c") )
				s += "/* split */";   
			// Emit just a prototype now and request definition later
			// Split out the definition of the instance for rendering later at Program scope
			if( !TreePtr<Uninitialised>::DynamicCast(o->initialiser) )
				definitions.push(o);
		}		
		else
		{
			if( ReadArgs::use.count("c") )
				s += "/* no split */";   
			// Emit the whole lot in-line
			s += RenderInitialisation( o->initialiser );							
		}
	}
	return s;
}


// Non-const static objects in records and functions 
// get split into a part that goes into the record (main line of rendering) and
// a part that goes separately (definitions gets appended at the very end).
// Do all functions, since SortDecls() ignores function bodies for dep analysis
bool CppRender::ShouldSplitInstance( TreePtr<Instance> o, Syntax::Production , Syntax::Policy policy ) 
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
        //if( surround_prod==Syntax::Production::DECLARATION )
	//		return false;
			
        if( policy.split_bulky_statics )
		{
			// we're a field of a record
			if( TreePtr<Static> s = DynamicTreePtrCast<Static>(o) )
			{
				if( DynamicTreePtrCast<Const>(s->constancy) && DynamicTreePtrCast<Numeric>( o->type ) )
					return false;

				return true;                
			}
		}

        return false;
    }
}


string CppRender::RenderMacroDeclaration( TreePtr<MacroDeclaration> md, Syntax::Production surround_prod )
{
	(void)surround_prod;	
    // ---- Proto ----
	string s = DoRender( md->identifier, Syntax::Production::POSTFIX );
	list<string> renders;
	for( TreePtr<Node> node : md->arguments )
		renders.push_back( DoRender(node, Syntax::Production::COMMA_SEP) );
	s += Join(renders, ", ", "(", ")");
	
	// ---- Initialisation ----	    
    return s + RenderInitialisation( md->initialiser );
}


string CppRender::RenderRecordProto( TreePtr<Record> record, Syntax::Policy policy )
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
        throw Syntax::Unimplemented();

    // Name of the record
    s += " " + DoRender( record->identifier, Syntax::Production::PURE_IDENTIFIER, policy); // Don't want scope resolution when declaring
    
    return s;
}


string CppRender::RenderPreProcDecl( TreePtr<PreProcDecl> ppd, Syntax::Production surround_prod ) try
{
	(void)surround_prod;
    if( auto si = TreePtr<SystemInclude>::DynamicCast(ppd) )
        return "#include <" + si->filename->GetString() + ">";
    else if( auto si = TreePtr<LocalInclude>::DynamicCast(ppd) )
        return "#include " + si->filename->GetRender(this, Syntax::Production::SPACE_SEP_PRE_PROC, default_policy);
    else
        return RenderNodeExplicit( ppd, surround_prod, default_policy );     
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderRecordBody( TreePtr<Record> record ) 
{
	string s;
	
	// Base classes
	if( TreePtr<InheritanceRecord> ir = DynamicTreePtrCast< InheritanceRecord >(record) )
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
				s += DoRender( b->access, Syntax::Production::TERMINAL ) + " ";
				s += DoRender( b->record, Syntax::Production::RESOLVER);
			}
		}
	}

	// Members
	s += "\n{ // memb\n";
	TreePtr<AccessSpec> a = record->GetInitialAccess();
	if( a )
		s += RenderDeclScope( record, type_index(typeid(*a)) );          
	else
		s += RenderEnumBodyScope( record ); 
		
	s += "}";
	return s;
}


string CppRender::RenderDeclaration( TreePtr<Declaration> declaration, Syntax::Production surround_prod, Syntax::Policy policy ) try
{
    TRACE();
	(void)surround_prod;

    if( TreePtr<Typedef> t = DynamicTreePtrCast< Typedef >(declaration) )
    {
        Syntax::Production starting_declarator_prod = Syntax::Production::PURE_IDENTIFIER;
        auto id = DoRender( t->identifier, starting_declarator_prod);
        return "typedef " + RenderTypeAndDeclarator( t->type, id, starting_declarator_prod, Syntax::Production::SPACE_SEP_DECLARATION );
    }
    else if( TreePtr<Record> record = DynamicTreePtrCast< Record >(declaration) )
    {
        // Prototype of the record
        try
        {
			string s = RenderRecordProto( record, policy );        
			if( !policy.force_incomplete_records )
			{
				s += RenderRecordBody( record );
				s = '\n' + s + '\n';
			}
			return s;
		}
		catch( Syntax::Unimplemented &e )
		{
		}
    }
    else if( TreePtr<LabelDeclaration> l = DynamicTreePtrCast<LabelDeclaration>(declaration) )
        return DoRender( l->identifier, Syntax::Production::PURE_IDENTIFIER) + ":"; 
    
    return Render::Dispatch( declaration, surround_prod, policy );
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderStatement( TreePtr<Statement> statement, Syntax::Production surround_prod, Syntax::Policy policy ) try
{
    (void)surround_prod;
    TRACE();
    ASSERT( statement );
    //printf( "%s %d things\n", typeid(*statement).name(), statement->Itemise().size() );
    if( TreePtr<Declaration> d = DynamicTreePtrCast< Declaration >(statement) )
        return RenderDeclaration( d, surround_prod, policy );
    else if( TreePtr<Expression> e = DynamicTreePtrCast< Expression >(statement) )
        return DoRender( e, surround_prod);
    else if( TreePtr<Return> es = DynamicTreePtrCast<Return>(statement) )
        return "return " + DoRender( es->return_value, Syntax::Production::SPACE_SEP_STATEMENT);
    else if( TreePtr<Goto> g = DynamicTreePtrCast<Goto>(statement) )
    {
        if( TreePtr<SpecificLabelIdentifier> li = DynamicTreePtrCast< SpecificLabelIdentifier >(g->destination) )
            return "goto " + DoRender( li, Syntax::Production::SPACE_SEP_STATEMENT).substr(2);  // regular goto REMOVE THE &&
        else
            return "goto *" + DoRender( g->destination, Syntax::Production::PREFIX); // goto-a-variable (GCC extension)
    }
    else if( TreePtr<If> i = DynamicTreePtrCast<If>(statement) )
    {
        bool has_else_clause = !DynamicTreePtrCast<Nop>(i->body_else); // Nop means no else clause
        string s;
        s += "if( " + DoRender( i->condition, Syntax::Production::CONDITION) + " )\n";
        // The choice of production here causes then "else" ambuguity to be resolved.
        s += DoRender( i->body, has_else_clause ? Syntax::Production::STATEMENT_HIGH : Syntax::Production::STATEMENT_LOW);
        if( has_else_clause )  
            s += "else\n" + DoRender( i->body_else, Syntax::Production::STATEMENT_LOW);
        return s;
    }
    else if( TreePtr<While> w = DynamicTreePtrCast<While>(statement) )
        return "while( " + 
               DoRender( w->condition, Syntax::Production::CONDITION) + " )\n" +
               DoRender( w->body, surround_prod);
    else if( TreePtr<Do> d = DynamicTreePtrCast<Do>(statement) )
        return "do\n" +
               DoRender( d->body, Syntax::Production::STATEMENT_LOW) +
               "while( " + DoRender( d->condition, Syntax::Production::CONDITION) + " )";
    else if( TreePtr<For> f = DynamicTreePtrCast<For>(statement) )
        return "for( " + 
               DoRender( f->initialisation, Syntax::Production::STATEMENT_LOW) + 
               DoRender( f->condition, Syntax::Production::STATEMENT_LOW) + 
               DoRender( f->increment, Syntax::Production::BOTTOM_EXPR) + " )\n" +
               DoRender( f->body, surround_prod);
    else if( TreePtr<Switch> s = DynamicTreePtrCast<Switch>(statement) )
        return "switch( " + DoRender( s->condition, Syntax::Production::CONDITION) + " )\n" +
               DoRender( s->body, surround_prod);
    else if( TreePtr<Case> c = DynamicTreePtrCast<Case>(statement) )
        return "case " + DoRender( c->value, Syntax::Production::SPACE_SEP_STATEMENT) + ":";
    else if( TreePtr<RangeCase> rc = DynamicTreePtrCast<RangeCase>(statement) )
        // GCC extension: assume that ... is part of the case statement, and can boot the expressions.
        return "case " + 
               DoRender( rc->value_lo, Syntax::Production::SPACE_SEP_STATEMENT) + 
               " ... " + 
               DoRender( rc->value_hi, Syntax::Production::SPACE_SEP_STATEMENT) + 
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
        return Render::Dispatch( statement, surround_prod, policy );
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderConstructorInitList( Sequence<Statement> spe ) try
{
    list<string> ls; 
    for( TreePtr<Statement> st : spe )
        ls.push_back( "    " + DoRender( st, Syntax::Production::COMMA_SEP, default_policy ) );		
    return Join(ls, ",\n");
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
            s += RenderNodeExplicit( pe, Syntax::Production::ASSIGN, default_policy );
            continue;
        }
        // We're really declaring the id, and don't want scope resolution
        s += DoRender( o->identifier, Syntax::Production::PURE_IDENTIFIER); 
        
        auto ei = TreePtr<Expression>::DynamicCast( o->initialiser );
        if( !ei )
        {
            s += RenderNodeExplicit( o->initialiser, Syntax::Production::INITIALISER, default_policy );
            continue;
        }       
        s += DoRender( ei, Syntax::Production::INITIALISER);

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
		renders.push_back( DoRender( pe, Syntax::Production::COMMA_SEP ) );
    return Join(renders, ", ", "{", "}"); // TODO now only used in operaotr new, where the {} looks wrong
}
DEFAULT_CATCH_CLAUSE


string CppRender::MaybeRenderFieldAccess( TreePtr<Declaration> declaration,
                                          type_index *current_access )
{
    ASSERT( current_access );
    string s;
    
    // Decide access spec for this declaration (explicit if instance, 
    // otherwise force to Public because decls don't have an access spec). TODO fix this, #877
    TreePtr<AccessSpec> this_access = MakeTreeNode<Public>();
	if( TreePtr<Field> f = DynamicTreePtrCast<Field>(declaration) )
		this_access = f->access;

	Syntax::Policy policy = default_policy;
	policy.current_access = *current_access;
    s = DoRender( this_access, Syntax::Production::BARE_DECLARATION, policy );
    *current_access = type_index(typeid(*this_access));
    
    return s;  
}                                 


string CppRender::RenderDeclScope( TreePtr<DeclScope> decl_scope,
                                   type_index init_access ) try
{
    TRACE();
    Syntax::Policy decl_scope_policy = default_policy;
    if( DynamicTreePtrCast<Record>(decl_scope) )
    {
		decl_scope_policy.split_bulky_statics = true; // Our scope is a record body
		decl_scope_policy.permit_static_keyword = true; // Our scope is a record body
	}
    Sequence<Declaration> sorted = SortDecls( decl_scope->members, true, unique_identifier_names );

	queue<TreePtr<Declaration>> require_complete;
	
    // Emit preprocs and an incomplete for each record 
    string s;
    for( TreePtr<Declaration> pd : sorted )
    {       
        if( auto ppd = DynamicTreePtrCast<PreProcDecl>(pd) )
        {
            s += DoRender( ppd, Syntax::Production::DECLARATION ) + "\n";
            continue;
        }
        
        if( DynamicTreePtrCast<Record>(pd) && !DynamicTreePtrCast<Enum>(pd) ) 
        {    
			// A record within our scope
			s += MaybeRenderFieldAccess( pd, &init_access );

			Syntax::Policy record_policy = decl_scope_policy;
			record_policy.force_incomplete_records = true; 
			s += DoRender( pd, Syntax::Production::DECLARATION, record_policy ); 
		}
		
		require_complete.push( pd );
    }
    
    // Emit the actual definitions, sorted for dependencies
    while( !require_complete.empty()  )
    {
        TreePtr<Declaration> d = require_complete.front();
        require_complete.pop();       		
        s += MaybeRenderFieldAccess( d, &init_access );        
        s += DoRender( d, Syntax::Production::DECLARATION, decl_scope_policy );
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
            s += RenderNodeExplicit( d, Syntax::Production::BARE_DECLARATION, default_policy );
            continue;
        }
        Syntax::Production starting_declarator_prod = Syntax::Production::PURE_IDENTIFIER;
        string name = DoRender( o->identifier, starting_declarator_prod);
        s += RenderTypeAndDeclarator( o->type, name, starting_declarator_prod, Syntax::Production::BARE_DECLARATION, false );
            
        first = false;
    }
    return s;
}
DEFAULT_CATCH_CLAUSE
