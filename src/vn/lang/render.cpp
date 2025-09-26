#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
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
using namespace SCTree;
using namespace VN;

// Don't like the layout of rendered code?
// There's a .clang-format in repo root.

// TODO indent back to previous level at end of string
#define ERROR_UNKNOWN(V) \
    string( "«" ) + \
    string( V ) + \
    string( " not supported in " ) + \
    string( __func__ ) + \
    string( "()»" );

#define ERROR_UNSUPPORTED(P) \
    ERROR_UNKNOWN( P ? typeid(*P).name() : "<nullptr>" );

// For #400 make methods that return strings try-functions
// and use this for the catch clause.
#define DEFAULT_CATCH_CLAUSE \
    catch( const ::Mismatch &e ) \
    { \
        return RenderMismatchException( __func__, e ); \
    } 


#define SYSTEMC_MARKER(V) SSPrintf( " // SystemC %s:%d\n", __FILE__, __LINE__ )



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

// Note: this does not modify the program tree, and that can be checked by 
// defining TEST_FOR_UNMODIFIED_TREE and retesting everything.
//#define TEST_FOR_UNMODIFIED_TREE
string Render::RenderToString( TreePtr<Node> root )
{   
    DefaultTransUtils utils(root);
    Render::Kit kit { &utils };
    
#ifdef TEST_FOR_UNMODIFIED_TREE    
    temp_old_root = root;
    root = Duplicate::DuplicateSubtree(root);
#endif
    
    // Top level is expected to be a scope of some kind
    root_scope = dynamic_pointer_cast<Scope>(root);
    if( !root_scope )
		return ERROR_UNKNOWN( Trace(root) );

#ifdef TEST_FOR_UNMODIFIED_TREE   
    bool before = sc(root, temp_old_root);  
#endif
    
    // Track scopes for name resolution
    AutoPush< TreePtr<Node> > cs( scope_stack, root_scope );
    
	// pre-pass to fill in backing_ordering. It would be better to tell 
	// it that it's doing a pre-pass so it can adapt its behaviour.
    RenderScope( kit, root_scope ); 
    deferred_decls.clear();

    // Make the identifiers unique
    unique_ids = UniquifyIdentifiers::UniquifyAll( kit, root );

    string s;

    if( IsSystemC( kit, root_scope ) )
        s += "#include \"isystemc.h\"\n\n";

    s += RenderScope( kit, root_scope ); // gets the .hpp stuff directly

    s += deferred_decls; // these could go in a .cpp file

#ifdef TEST_FOR_UNMODIFIED_TREE   
    ASSERT( sc(root, temp_old_root) == before );        
#endif 
	return s;
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


bool Render::IsSystemC( const Render::Kit &, TreePtr<Node> root )
{ 
    Walk e(root, nullptr, nullptr);
    for( const TreePtrInterface &n : e )
        if( dynamic_pointer_cast<SCConstruct>((TreePtr<Node>)n) )
            return true;
    return false;
}


string Render::RenderLiteral( const Render::Kit &kit, TreePtr<Literal> sp ) try
{
	(void)kit;
    return Sanitise( sp->GetToken() );
}
DEFAULT_CATCH_CLAUSE


string Render::RenderIdentifier( const Render::Kit &kit, TreePtr<Identifier> id ) try
{
	(void)kit;
    string ids;
    if( id )
    {
        if( TreePtr<SpecificIdentifier> ii = DynamicTreePtrCast<SpecificIdentifier>( id ) )
        {
            if( unique_ids.count(ii) == 0 )
                return ERROR_UNKNOWN( SSPrintf("identifier %s missing from unique_ids", ii->GetToken().c_str() ) );
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


string Render::RenderScopePrefix( const Render::Kit &kit, TreePtr<Identifier> id, Syntax::Production surround_prod ) try
{
	TreePtr<Node> scope;
	try
	{
		scope = GetScope( root_scope, id );
	}
 	catch( ScopeNotFoundMismatch & )
 	{
		// Assume that specific identifiers added by SC lowering are all in global scope eg from a #include
		return "";
	}
       
    //TRACE("%p %p %p\n", program.get(), scope.get(), scope_stack.top().get() );
    if( scope == scope_stack.top() )
        return ""; // local scope
    else if( scope == root_scope )
        return " ::";
    else if( TreePtr<Enum> e = DynamicTreePtrCast<Enum>( scope ) ) // <- for enum
        return RenderScopePrefix( kit, e->identifier, surround_prod );    // omit scope for the enum itself
    else if( TreePtr<Record> r = DynamicTreePtrCast<Record>( scope ) ) // <- for class, struct, union
        return RenderScopedIdentifier( kit, r->identifier, Syntax::Production::SCOPE_RES ) + "::";
    else if( DynamicTreePtrCast<CallableParams>( scope ) ||  // <- this is for params
             DynamicTreePtrCast<Compound>( scope ) ||    // <- this is for locals in body
             DynamicTreePtrCast<StatementExpression>( scope ) )    // <- this is for locals in body
        return "";
    else
        return ERROR_UNSUPPORTED( scope );
}
DEFAULT_CATCH_CLAUSE


string Render::RenderScopedIdentifier( const Render::Kit &kit, TreePtr<Identifier> id, Syntax::Production surround_prod ) try
{
	if( Syntax::GetPrecedence(Syntax::Production::SCOPE_RES) < Syntax::GetPrecedence(surround_prod) )
		return "(" + RenderScopedIdentifier(kit, id, Syntax::Production::BOOT_EXPR) + ")";

    string s = RenderScopePrefix( kit, id, surround_prod );   
    s += RenderIdentifier( kit, id );

    TRACE("Render scoped identifier %s\n", s.c_str() );
    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderIntegralType( const Render::Kit &kit, TreePtr<Integral> type, string object ) try
{
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
    bool bitfield = false;
    if( width == TypeDb::char_bits )
        s += "char";
    else if( width == TypeDb::integral_bits[clang::DeclSpec::TSW_unspecified] )
        s += "int";
    else if( width == TypeDb::integral_bits[clang::DeclSpec::TSW_short] )
        s += "short";
    else if( width == TypeDb::integral_bits[clang::DeclSpec::TSW_long] )
        s += "long";
    else if( width == TypeDb::integral_bits[clang::DeclSpec::TSW_longlong] )
        s += "long long";
    else   
    {
        // unmatched defaults to int for bitfields
        s += "int";
        bitfield = true;
    }

    s += " " + object;

    if( bitfield )
    {
       char b[100];
       sprintf(b, ":%" PRId64, width);
       s += b;
    }

    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderFloatingType( const Render::Kit &kit, TreePtr<Floating> type ) try
{
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


string Render::RenderType( const Render::Kit &kit, TreePtr<Type> type, string object, Syntax::Production object_prod, 
                           bool constant ) try
{
    Syntax::Production surround_prod = type->GetOperandInDeclaratorProduction();
	ASSERT( surround_prod != Syntax::Production::UNDEFINED )("Rendering type: ")(type)(" in production %d",(int)surround_prod)(" got no surrounding production\n");
	ASSERT( Syntax::GetPrecedence(surround_prod) <= Syntax::GetPrecedence(Syntax::Production::PARENTHESISED) ); // Can't satisfy this production's precedence demand using parentheses
	ASSERT( Syntax::GetPrecedence(object_prod) >= Syntax::GetPrecedence(Syntax::Production::BOOT_EXPR) ); // Can't puth this node into parentheses
	bool parenthesise = Syntax::GetPrecedence(object_prod) < Syntax::GetPrecedence(surround_prod);	
    // Apply to object rather than recursing, because this is declarator
    if( parenthesise )
		object = "(" + object + ")";
    string sobject;
    if( !object.empty() )
        sobject = " " + object;
                
    string const_str = constant?"const ":"";

    TRACE();
    if( TreePtr<Integral> i = DynamicTreePtrCast< Integral >(type) )
        return const_str + RenderIntegralType( kit, i, object );
    if( TreePtr<Floating> f = DynamicTreePtrCast< Floating >(type) )
        return const_str + RenderFloatingType( kit, f ) + sobject;
    else if( DynamicTreePtrCast< Void >(type) )
        return const_str + "void" + sobject;
    else if( DynamicTreePtrCast< Boolean >(type) )
        return const_str + "bool" + sobject;
    else if( TreePtr<Constructor> c = DynamicTreePtrCast< Constructor >(type) )
        return object + "(" + RenderParams(kit, c) + ")" + const_str;
    else if( TreePtr<Destructor> f = DynamicTreePtrCast< Destructor >(type) )
        return object + "()" + const_str;
    else if( TreePtr<Function> f = DynamicTreePtrCast< Function >(type) )
        return RenderType( kit, f->return_type, object + "(" + RenderParams(kit, f) + ")" + const_str, Syntax::Production::POSTFIX );
    else if( TreePtr<Process> f = DynamicTreePtrCast< Process >(type) )
        return "void " + object + "()" + const_str + SYSTEMC_MARKER();
    else if( TreePtr<Pointer> p = DynamicTreePtrCast< Pointer >(type) )
        return RenderType( kit, p->destination, string(DynamicTreePtrCast<Const>(p->constancy)?"const ":"") + "*" + const_str + object, Syntax::Production::PREFIX, false ); // TODO Pointer node to indicate constancy of pointed-to object - would go into this call to RenderType
    else if( TreePtr<Reference> r = DynamicTreePtrCast< Reference >(type) )
        return RenderType( kit, r->destination, string(DynamicTreePtrCast<Const>(p->constancy)?"const ":"") + "&" + const_str + object, Syntax::Production::PREFIX );
    else if( TreePtr<Array> a = DynamicTreePtrCast< Array >(type) )
        return RenderType( kit, 
                           a->element, 
                           object + "[" + RenderExpression(kit, a->size, Syntax::Production::BOOT_EXPR) + "]", 
                           Syntax::Production::POSTFIX,
                           constant );
    else if( TreePtr<Typedef> t = DynamicTreePtrCast< Typedef >(type) )
        return const_str + RenderIdentifier(kit, t->identifier) + sobject;
    else if( TreePtr<SpecificTypeIdentifier> ti = DynamicTreePtrCast< SpecificTypeIdentifier >(type) )
        return const_str + RenderScopedIdentifier(kit, ti, Syntax::Production::BOOT_EXPR) + sobject;
    else if( shared_ptr<SCNamedIdentifier> sct = dynamic_pointer_cast< SCNamedIdentifier >(type) )
        return const_str + sct->GetToken() + sobject;
    else if( dynamic_pointer_cast<Labeley>(type) )
        return const_str + "void *" + object;
    else
        return ERROR_UNSUPPORTED(type);
}
DEFAULT_CATCH_CLAUSE


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


string Render::RenderOperator( const Render::Kit &kit, TreePtr<Operator> op, Sequence<Expression> &operands ) try
{
    ASSERT(op);
    string s;
    Sequence<Expression>::iterator operands_it = operands.begin();
    if( DynamicTreePtrCast< MakeArray >(op) )
    {
        s = "{ " + RenderOperandSequence( kit, operands ) + " }";
    }
    else if( DynamicTreePtrCast< ConditionalOperator >(op) )
    {
        s = RenderExpression( kit, *operands_it, Syntax::BoostPrecedence(Syntax::Production::CONDITIONAL) ) + " ? ";
        ++operands_it;
        // Middle expression boots parser
        s += RenderExpression( kit, *operands_it, Syntax::Production::BOOT_EXPR ) + " : ";
        ++operands_it;
        s += RenderExpression( kit, *operands_it, Syntax::Production::CONDITIONAL );           
    }
    else if( DynamicTreePtrCast< Subscript >(op) )
    {
        s = RenderExpression( kit, *operands_it, Syntax::Production::POSTFIX ) + "[";
        ++operands_it;
        s += RenderExpression( kit, *operands_it, Syntax::Production::BOOT_EXPR ) + "]";
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
        s = RenderExpression( kit, *operands_it, prod_left ); \
        s += TEXT; \
        ++operands_it; \
        s += RenderExpression( kit, *operands_it, prod_right ); \
    }
#define PREFIX(TOK, TEXT, NODE_SHAPED, BASE, CAT, PROD, ASSOC) \
    else if( DynamicTreePtrCast<NODE_SHAPED>(op) ) \
    { \
        s = TEXT; \
        bool paren = false; \
        /* Prevent interpretation as a member function pointer literal */ \
        if( auto ao = TreePtr<AddressOf>::DynamicCast(op) ) \
			if( auto id = TreePtr<Identifier>::DynamicCast(*operands_it) ) \
			    paren = !RenderScopePrefix( kit, id, Syntax::Production::PROD ).empty(); \
        s += (paren?"(":"") + RenderExpression( kit, *operands_it, Syntax::Production::PROD) + (paren?")":""); \
    }
#define POSTFIX(TOK, TEXT, NODE_SHAPED, BASE, CAT, PROD, ASSOC) \
    else if( DynamicTreePtrCast<NODE_SHAPED>(op) ) \
    { \
        s = RenderExpression( kit, *operands_it, Syntax::Production::PROD ); \
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


string Render::RenderMapArgs( const Render::Kit &kit, TreePtr<Call> call ) try
{
    string s;

    // If CallableParams, generate some arguments, resolving the order using the original function type
    TreePtr<Node> ctype = TypeOf::instance(call->callee, root_scope).GetTreePtr();

    s += "(";
	
    ASSERT( ctype );
    if( TreePtr<CallableParams> cp = DynamicTreePtrCast<CallableParams>(ctype) )
        s += RenderMapInOrder( kit, call, cp );

    s += ")";
    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderCall( const Render::Kit &kit, TreePtr<Call> call ) try
{
    string s;

    // Render the expression that resolves to the function name unless this is
    // a constructor call in which case just the name of the thing being constructed.
    if( TreePtr<Expression> base = TypeOf::instance.TryGetConstructedExpression( kit, call ).GetTreePtr() )
        s += RenderExpression( kit, base, Syntax::Production::POSTFIX );
    else
        s += RenderExpression( kit, call->callee, Syntax::Production::POSTFIX );

    s += RenderMapArgs(kit, call);
    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderSysCall( const Render::Kit &kit, TreePtr<SysCall> call ) try
{
    string s;

    // Render the expression that resolves to the function name 
    s += RenderExpression( kit, call->callee, Syntax::Production::POSTFIX );

	// SysCall just has a sequence of arg expressions
    list<string> renders;
    for( TreePtr<Expression> e : call->operands )    
		renders.push_back( RenderExpression(kit, e, Syntax::Production::COMMA_SEP) );				
	s += Join(renders, ", ", "(", ")");

    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderExpression( const Render::Kit &kit, TreePtr<Initialiser> expression, Syntax::Production surround_prod ) try
{
	Syntax::Production node_ideal_prod = expression->GetMyProduction();
	// If current production has too-high precedence, boot back down using parentheses
	ASSERT( node_ideal_prod != Syntax::Production::UNDEFINED )("Rendering expression: ")(expression)(" in production %d",(int)surround_prod)(" got no ideal production\n");
	ASSERT( Syntax::GetPrecedence(surround_prod) <= Syntax::GetPrecedence(Syntax::Production::PARENTHESISED) ); // Can't satisfy this production's precedence demand using parentheses
	ASSERT( Syntax::GetPrecedence(node_ideal_prod) >= Syntax::GetPrecedence(Syntax::Production::BOOT_EXPR) ); // Can't puth this node into parentheses
	bool parenthesise = Syntax::GetPrecedence(node_ideal_prod) < Syntax::GetPrecedence(surround_prod);	
	if( parenthesise )
		return "(" + RenderExpression( kit, expression, Syntax::Production::BOOT_EXPR ) + ")";

    if( DynamicTreePtrCast< Uninitialised >(expression) )
        return string();
    else if( auto ce = DynamicTreePtrCast< StatementExpression >(expression) )
    {
        AutoPush< TreePtr<Node> > cs( scope_stack, ce );
        string s = "({ ";
        s += RenderScope( kit, ce ); // Must do this first to populate backing list
		for( TreePtr<Statement> st : ce->statements )    
			s += RenderStatement( kit, st );    
        return s + "})";
    }
    else if( auto li = DynamicTreePtrCast< SpecificLabelIdentifier >(expression) )
        return "&&" + RenderIdentifier( kit, li ); // label-as-variable (GCC extension)             
    else if( auto ii = DynamicTreePtrCast< SpecificInstanceIdentifier >(expression) )
        return RenderScopedIdentifier( kit, ii, surround_prod );
    else if( auto pot = DynamicTreePtrCast< SizeOf >(expression) )
        return "sizeof(" + RenderType( kit, pot->operand, "", Syntax::Production::ANONYMOUS ) + ")";               
    else if( auto pot = DynamicTreePtrCast< AlignOf >(expression) )
        return "alignof(" + RenderType( kit, pot->operand, "", Syntax::Production::ANONYMOUS ) + ")";
    else if( auto nco = DynamicTreePtrCast< NonCommutativeOperator >(expression) )
        return RenderOperator( kit, nco, nco->operands );           
    else if( auto co = DynamicTreePtrCast< CommutativeOperator >(expression) )
    {
        Sequence<Expression> seq_operands;
        // Operands are in collection, so sort them and put them in a sequence
        for( TreePtr<Node> o : sc.GetTreePtrOrdering(co->operands) )
            seq_operands.push_back( TreePtr<Expression>::DynamicCast(o) );
        return RenderOperator( kit, co, seq_operands );               
    }
    else if( auto c = DynamicTreePtrCast< Call >(expression) )
        return RenderCall( kit, c );
    else if( auto sc = DynamicTreePtrCast< SysCall >(expression) )
        return RenderSysCall( kit, sc );
    else if( auto n = DynamicTreePtrCast< New >(expression) )
        return string (DynamicTreePtrCast<Global>(n->global) ? "::" : "") +
               "new(" + RenderOperandSequence( kit, n->placement_arguments ) + ") " +
               RenderType( kit, n->type, "", Syntax::Production::ANONYMOUS ) +
               (n->constructor_arguments.empty() ? "" : "(" + RenderOperandSequence( kit, n->constructor_arguments ) + ")" );
    else if( auto d = DynamicTreePtrCast< Delete >(expression) )
        return string(DynamicTreePtrCast<Global>(d->global) ? "::" : "") +
               "delete" +
               (DynamicTreePtrCast<DeleteArray>(d->array) ? "[]" : "") +
               " " + RenderExpression( kit, d->pointer, Syntax::Production::PREFIX );
    else if( auto a = DynamicTreePtrCast< Lookup >(expression) )
        return RenderExpression( kit, a->base, Syntax::Production::POSTFIX ) + "." +
               RenderScopedIdentifier( kit, a->member, Syntax::BoostPrecedence(Syntax::Production::POSTFIX) );
    else if( auto c = DynamicTreePtrCast< Cast >(expression) )
        return "(" + RenderType( kit, c->type, "", Syntax::Production::ANONYMOUS ) + ")" +
               RenderExpression( kit, c->operand, Syntax::Production::PREFIX );
    else if( auto ro = DynamicTreePtrCast< MakeRecord >(expression) )
        return RenderMakeRecord( kit, ro );
    else if( auto l = DynamicTreePtrCast< Literal >(expression) )
        return RenderLiteral( kit, l );
    else if( DynamicTreePtrCast< This >(expression) )
        return "this";
    else if( auto dc = DynamicTreePtrCast<DeltaCount>(expression) ) 
        return dc->GetToken() + "()";
    else
        return ERROR_UNSUPPORTED(expression);
}
DEFAULT_CATCH_CLAUSE


string Render::RenderMakeRecord( const Render::Kit &kit, TreePtr<MakeRecord> ro ) try
{
    string s;

    // Get the record
    TreePtr<TypeIdentifier> id = DynamicTreePtrCast<TypeIdentifier>(ro->type);
    ASSERT(id);

    TreePtr<Record> r = GetRecordDeclaration(kit, id).GetTreePtr();

    s += "(";
    s += RenderType( kit, ro->type, "", Syntax::Production::ANONYMOUS );
    s += "){ ";
    s += RenderMapInOrder( kit, ro, r );
    s += " }";
    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderMapInOrder( const Render::Kit &kit, 
                                 TreePtr<MapOperator> ro,
                                 TreePtr<Node> key ) try
{	
	if( backing_ordering.count(key) == 0 )	
		return "«unknown ordering for "+Trace(key)+", OK in pre-pass»";        	

    string s;
    bool first = true;
    
    // Get a reference to the ordered list of members for this record from our backing list
    Sequence<Declaration> &sd = backing_ordering.at(key);
    for( TreePtr<Declaration> d : sd )
    {
        // We only care about instances...
        if( TreePtr<Instance> i = DynamicTreePtrCast<Instance>( d ) )
        {
            // ...and not function instances
            if( !DynamicTreePtrCast<Callable>( i->type ) )
            {
                // search init for matching member (TODO could avoid O(n^2) by exploiting the map)
                for( TreePtr<MapOperand> mi : ro->operands )
                {
                    if( i->identifier == mi->identifier )
                    {
                        if( !first )
                            s += ", ";
                        s += RenderExpression( kit, mi->value, Syntax::Production::COMMA_SEP );
                        first = false;
                    }
                }
            }
        }
    }
    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderAccess( const Render::Kit &kit, TreePtr<AccessSpec> current_access ) try
{
	(void)kit;
    if( DynamicTreePtrCast<Public>( current_access ) )
        return "public";
    else if( DynamicTreePtrCast<Private>( current_access ) )
        return "private";
    else if( DynamicTreePtrCast<Protected>( current_access ) )
        return "protected";
    else
        return ERROR_UNKNOWN("current_access spec");
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
        if( auto o = DynamicTreePtrCast< Call >(s) )
        {
            try
            {
                if( TypeOf::instance.TryGetConstructedExpression( kit, o ) )
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
                                    TreePtr<Instance> o, 
                                    bool out_of_line ) try
{
    string s;
    string name;
    bool constant=false;

    ASSERT(o->type);

    if( TreePtr<Static> st = DynamicTreePtrCast<Static>(o) )
        if( DynamicTreePtrCast<Const>(st->constancy) )
            constant = true;
    if( TreePtr<Field> f = DynamicTreePtrCast<Field>(o) )
        if( DynamicTreePtrCast<Const>(f->constancy) )
            constant = true;

    if( out_of_line )
        name += RenderScopePrefix(kit, o->identifier, Syntax::Production::SCOPE_RES);
    else // in-line
        s += RenderStorage(kit, o);
 
    TreePtr<Constructor> con = DynamicTreePtrCast<Constructor>(o->type);
    TreePtr<Destructor> de = DynamicTreePtrCast<Destructor>(o->type);
    if( con || de )
    {
        TreePtr<Record> rec = DynamicTreePtrCast<Record>( GetScope( root_scope, o->identifier ) );
        ASSERT( rec );        
        name += (de ? "~" : "");
        name += RenderIdentifier(kit, rec->identifier);
    }
    else
    {
        name += RenderIdentifier(kit, o->identifier);
    }

    s += RenderType( kit, o->type, name, Syntax::Production::SCOPE_RES, constant );

	return s;
} 
DEFAULT_CATCH_CLAUSE


string Render::RenderInstance( const Render::Kit &kit, TreePtr<Instance> o, 
                               bool out_of_line ) try
{
    string s = RenderInstanceProto( kit, o, out_of_line );
	
	// SystemC: declaring instance of module at top level.
    // If object was declared as a module instance, bodge in a name as a constructor parameter
    // But not for fields - they need an init list, done in RenderScope()
    if( !DynamicTreePtrCast<Field>(o) )
    {
        if( TreePtr<TypeIdentifier> tid = DynamicTreePtrCast<TypeIdentifier>(o->type) ) try
        {
            if( TreePtr<Record> r = GetRecordDeclaration(kit, tid).GetTreePtr() )
            {
				if( DynamicTreePtrCast<Module>(r) )
                {
                    s += "(\"" + RenderIdentifier(kit, o->identifier) + "\")" + ";" + SYSTEMC_MARKER();
                    return s;
                }
			}
		}
		catch(DeclarationOf::DeclarationNotFound &)
		{
			// No decl so probably a system type. 				
		}			
    }
    
    if( DynamicTreePtrCast<Uninitialised>(o->initialiser) )
        return s + ";\n"; // Don't render any initialiser    
    
    if( DynamicTreePtrCast<Callable>(o->type) )
    {
        // Establish the scope of the function
        AutoPush< TreePtr<Node> > cs( scope_stack, GetScope( root_scope, o->identifier ) );

        // Put the contents of the body into a Compound-like form even if there's only one
        // Statement there - this is because we will wrangle with them later
        Sequence<Statement> code;
        Collection<Declaration> members;
        if( TreePtr<Compound> comp = DynamicTreePtrCast<Compound>(o->initialiser) )
        {
            members = comp->members;
            code = comp->statements;
        }
        else if( TreePtr<Statement> st = DynamicTreePtrCast<Statement>(o->initialiser) )
            code.push_back( st );
        else
            s += ERROR_UNSUPPORTED(o->initialiser);

        // Seperate the statements into constructor initialisers and "other stuff"
        Sequence<Statement> inits;
        Sequence<Statement> remainder;
        ExtractInits( kit, code, inits, remainder );

        // Render the constructor initialisers if there are any
        if( !inits.empty() )
        {
            s += " :\n";
            s += RenderConstructorInitList( kit, inits );
        }

        // Render the other stuff as a Compound so we always get {} in all cases
        auto r = MakeTreeNode<Compound>();
        r->members = members;
        r->statements = remainder;
        s += "\n" + RenderStatement(kit, r);
        
        // Surround functions with blank lines        
        return '\n' + s + '\n';
    }
    
    if( TreePtr<Expression> ei = DynamicTreePtrCast<Expression>( o->initialiser ) )
	{
		// Attempt direct initialisation
		if( auto call = DynamicTreePtrCast<Call>( ei ) )		
			if( TypeOf::instance.TryGetConstructedExpression( kit, call ).GetTreePtr() )		
				return s + RenderMapArgs(kit, call) + ";\n";
					
		// Render expression with an assignment
		AutoPush< TreePtr<Node> > cs( scope_stack, GetScope( root_scope, o->identifier ) );		
		return s + " = " + RenderExpression(kit, ei, Syntax::Production::ASSIGN) + ";\n";
	}

    return s + ERROR_UNSUPPORTED(o->initialiser);
}
DEFAULT_CATCH_CLAUSE


// Non-const static objects in records and functions 
// get split into a part that goes into the record (main line of rendering) and
// a part that goes separately (deferred_decls gets appended at the very end).
// Do all functions, since SortDecls() ignores function bodies for dep analysis
bool Render::ShouldSplitInstance( const Render::Kit &kit, TreePtr<Instance> o ) 
{
	(void)kit;
    bool isfunc = !!DynamicTreePtrCast<Callable>( o->type );
    bool isnumber = !!DynamicTreePtrCast<Numeric>( o->type );       

    bool split_var = false;
    if( TreePtr<Static> s = DynamicTreePtrCast<Static>(o) )
        if( DynamicTreePtrCast<NonConst>(s->constancy) || !isnumber )
            split_var = true;

    return ( DynamicTreePtrCast<Record>( scope_stack.top() ) &&
               split_var ) || 
               isfunc;
}


string Render::RenderRecordProto( const Render::Kit &kit, TreePtr<Record> record )
{
	string s;
	shared_ptr<SCNamedRecord> scr = dynamic_pointer_cast< SCNamedRecord >(record);
	if( DynamicTreePtrCast< Class >(record) || scr )
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
	s += " " + RenderIdentifier(kit, record->identifier);
	
	return s;
}


string Render::RenderDeclaration( const Render::Kit &kit, TreePtr<Declaration> declaration ) try
{
    TRACE();
    string s;

    if( TreePtr<Instance> o = DynamicTreePtrCast<Instance>(declaration) )
    {
        if( ShouldSplitInstance(kit, o) )
        {
            s += RenderInstanceProto( kit, o, false ) + ";\n";
            {
                AutoPush< TreePtr<Node> > cs( scope_stack, root_scope );
                deferred_decls += string("\n") + RenderInstance( kit, o, true );
            }
        }
        else
        {
            // Otherwise, render everything directly using the default settings
            s += RenderInstance( kit, o, false );
        }
    }
    else if( TreePtr<Typedef> t = DynamicTreePtrCast< Typedef >(declaration) )
    {
        s += "typedef " + RenderType( kit, t->type, RenderIdentifier(kit, t->identifier), Syntax::Production::IDENTIFIER ) + ";\n";
    }
    else if( TreePtr<Record> r = DynamicTreePtrCast< Record >(declaration) )
    {
        shared_ptr<SCNamedRecord> scr = dynamic_pointer_cast< SCNamedRecord >(r);

        // Prototype of the record
		s += RenderRecordProto( kit, r );

 	    // Base classes
		if( TreePtr<InheritanceRecord> ir = DynamicTreePtrCast< InheritanceRecord >(declaration) )
		{
			if( !ir->bases.empty() || scr )
			{
				s += " : ";
				bool first=true;
				if( scr )
				{
					first = false;
					s += RenderAccess(kit, MakeTreeNode<Public>()) + " ";
					s += scr->GetToken();
				}
				for( TreePtr<Node> bn : sc.GetTreePtrOrdering(ir->bases) )
				{
					if( !first )
						s += ", ";
					first=false;
					auto b = TreePtr<Base>::DynamicCast(bn);
					ASSERT( b );
					s += RenderAccess(kit, b->access) + " ";
					s += RenderIdentifier(kit, b->record);
				}
			}
		}

		// Members
		AutoPush< TreePtr<Node> > cs( scope_stack, r );
		s += "\n{\n";
        TreePtr<AccessSpec> a;
        if( DynamicTreePtrCast< Class >(r) || scr )
            a = MakeTreeNode<Private>();
        else if( DynamicTreePtrCast< Struct >(r) )
            a = MakeTreeNode<Public>();
        else if( DynamicTreePtrCast< Union >(r) )
            a = MakeTreeNode<Public>();
        else if( DynamicTreePtrCast< Enum >(r) )
            a = nullptr;
        else
            return ERROR_UNSUPPORTED(declaration);

		if( a )
			s += RenderScope( kit, r, a );			
		else
			s += RenderEnumBody( kit, r->members );
			
		s += "};\n";
        // Add blank lines before and after
        s = '\n' + s + '\n';
    }
    else if( TreePtr<Label> l = DynamicTreePtrCast<Label>(declaration) )
        return RenderIdentifier(kit, l->identifier) + ":;\n"; // need ; after a label in case last in compound block
    else
        s += ERROR_UNSUPPORTED(declaration);

    TRACE();
    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderStatement( const Render::Kit &kit, TreePtr<Statement> statement ) try
{
    TRACE();
    if( !statement )
        return ";\n"; // TODO nasty, should assert not NULL in all of these in fact
    //printf( "%s %d things\n", typeid(*statement).name(), statement->Itemise().size() );
    if( TreePtr<Declaration> d = DynamicTreePtrCast< Declaration >(statement) )
        return RenderDeclaration( kit, d );
    else if( TreePtr<Compound> c = DynamicTreePtrCast< Compound >(statement) )
    {
        AutoPush< TreePtr<Node> > cs( scope_stack, c );
        string s = "{\n";
        s += RenderScope( kit, c ); // Must do this first to populate backing list
		for( TreePtr<Statement> st : c->statements )    
			s += RenderStatement( kit, st );    
        return s + "}\n";
    }
    else if( TreePtr<Expression> e = DynamicTreePtrCast< Expression >(statement) )
        return RenderExpression(kit, e, Syntax::Production::BOOT_EXPR) + ";\n";
    else if( TreePtr<Return> es = DynamicTreePtrCast<Return>(statement) )
        return "return " + RenderExpression(kit, es->return_value, Syntax::Production::BOOT_EXPR) + ";\n";
    else if( TreePtr<Goto> g = DynamicTreePtrCast<Goto>(statement) )
    {
        if( TreePtr<SpecificLabelIdentifier> li = DynamicTreePtrCast< SpecificLabelIdentifier >(g->destination) )
            return "goto " + RenderIdentifier(kit, li) + ";\n";  // regular goto
        else
            return "goto *" + RenderExpression(kit, g->destination, Syntax::Production::PREFIX) + ";\n"; // goto-a-variable (GCC extension)
    }
    else if( TreePtr<If> i = DynamicTreePtrCast<If>(statement) )
    {
		bool else_clause = !DynamicTreePtrCast<Nop>(i->else_body); // Nop means no else clause
        string s;
        s += "if( " + RenderExpression(kit, i->condition, Syntax::Production::BOOT_EXPR) + " )\n";
        bool sub_if = !!DynamicTreePtrCast<If>(i->body);
        if( sub_if && else_clause )
             s += "{\n"; // Note: braces there to clarify else binding eg if(a) if(b) foo; else how_do_i_bind;
        s += RenderStatement(kit, i->body);
        if( sub_if && else_clause )
             s += "}\n";
        if( else_clause )  
            s += "else\n" +
                 RenderStatement(kit, i->else_body);
        return s;
    }
    else if( TreePtr<While> w = DynamicTreePtrCast<While>(statement) )
        return "while( " + 
               RenderExpression(kit, w->condition, Syntax::Production::BOOT_EXPR) + " )\n" +
               RenderStatement(kit, w->body);
    else if( TreePtr<Do> d = DynamicTreePtrCast<Do>(statement) )
        return "do\n" +
               RenderStatement(kit, d->body) +
               "while( " + RenderExpression(kit, d->condition, Syntax::Production::BOOT_EXPR) + " );\n";
    else if( TreePtr<For> f = DynamicTreePtrCast<For>(statement) )
        return "for( " + 
               RenderStatement(kit, f->initialisation ) + 
               RenderExpression(kit, f->condition, Syntax::Production::BOOT_EXPR) + "; "+ 
               RenderExpression(kit, f->increment, Syntax::Production::BOOT_EXPR) + " )\n" +
               RenderStatement(kit, f->body);
    else if( TreePtr<Switch> s = DynamicTreePtrCast<Switch>(statement) )
        return "switch( " + RenderExpression(kit, s->condition, Syntax::Production::BOOT_EXPR) + " )\n" +
               RenderStatement(kit, s->body);
    else if( TreePtr<Case> c = DynamicTreePtrCast<Case>(statement) )
        return "case " + RenderExpression(kit, c->value, Syntax::Production::BOOT_EXPR) + ":;\n";
    else if( TreePtr<RangeCase> rc = DynamicTreePtrCast<RangeCase>(statement) )
		// GCC extension: assume that ... is part of the case statement, and can boot the expressions.
        return "case " + 
               RenderExpression(kit, rc->value_lo, Syntax::Production::BOOT_EXPR) + 
               " ... " + 
               RenderExpression(kit, rc->value_hi, Syntax::Production::BOOT_EXPR) + 
               ":\n";
    else if( DynamicTreePtrCast<Default>(statement) )
        return "default:;\n";
    else if( DynamicTreePtrCast<Continue>(statement) )
        return "continue;\n";
    else if( DynamicTreePtrCast<Break>(statement) )
        return "break;\n";
    else if( DynamicTreePtrCast<Nop>(statement) )
        return ";\n";
    else if( TreePtr<WaitDynamic> c = DynamicTreePtrCast<WaitDynamic>(statement) ) 
        return c->GetToken() + "( " + RenderExpression(kit, c->event, Syntax::Production::COMMA_SEP) + " ); " + SYSTEMC_MARKER();
    else if( TreePtr<WaitStatic> c = DynamicTreePtrCast<WaitStatic>(statement) ) 
        return c->GetToken() + "(); " + SYSTEMC_MARKER();
    else if( TreePtr<WaitDelta> c = DynamicTreePtrCast<WaitDelta>(statement) )
        return c->GetToken() + "(SC_ZERO_TIME); " + SYSTEMC_MARKER();
    else if( TreePtr<NextTriggerDynamic> c = DynamicTreePtrCast<NextTriggerDynamic>(statement) ) 
        return c->GetToken() + "( " + RenderExpression(kit, c->event, Syntax::Production::COMMA_SEP) + " ); " + SYSTEMC_MARKER();
    else if( TreePtr<NextTriggerStatic> c = DynamicTreePtrCast<NextTriggerStatic>(statement) ) 
        return c->GetToken() + "(); " + SYSTEMC_MARKER();
    else if( TreePtr<NextTriggerDelta> c = DynamicTreePtrCast<NextTriggerDelta>(statement) ) 
        return c->GetToken() + "(SC_ZERO_TIME); " + SYSTEMC_MARKER();
    else if( TreePtr<TerminationFunction> tf = DynamicTreePtrCast<TerminationFunction>(statement) )
        return tf->GetToken() + "( " + RenderExpression(kit, tf->code, Syntax::Production::COMMA_SEP) + " ); " + SYSTEMC_MARKER();
    else if( TreePtr<NotifyImmediate> n = DynamicTreePtrCast<NotifyImmediate>(statement) )
        return RenderExpression( kit, n->event, Syntax::Production::PREFIX ) + "." + n->GetToken() + "(); " + SYSTEMC_MARKER();
    else if( TreePtr<NotifyDelta> n = DynamicTreePtrCast<NotifyDelta>(statement) )
        return RenderExpression( kit, n->event, Syntax::Production::PREFIX ) + "." + n->GetToken() + "(SC_ZERO_TIME); " + SYSTEMC_MARKER();
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
            s += RenderExpression( kit, e, Syntax::Production::COMMA_SEP );
        else 
            s += ERROR_UNSUPPORTED(st);
        first = false;
    }
    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderEnumBody( const Render::Kit &kit, 
                               Collection<Declaration> spe ) try
{
    TRACE();
    string s;
    bool first = true;
    for( TreePtr<Declaration> pe : spe )
    {
		if( !first )
			s += ",\n";
			
	    auto o = TreePtr<Instance>::DynamicCast(pe);
	    if( !o )
	    {
			s += ERROR_UNSUPPORTED(pe);
			continue;
		}
	    s += RenderIdentifier(kit, o->identifier) + " = ";
	    
        auto ei = TreePtr<Expression>::DynamicCast( o->initialiser );
	    if( !ei )
	    {
			s += ERROR_UNSUPPORTED(o->initialiser);
			continue;
		}		
		s += RenderExpression(kit, ei, Syntax::Production::ASSIGN);

        first = false;    
    }
    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderOperandSequence( const Render::Kit &kit, 
                                      Sequence<Expression> spe ) try
{
    TRACE();
    string s;
    bool first = true;
    for( TreePtr<Expression> pe : spe )
    {
		if( !first )
			s += ", ";
        s += RenderExpression( kit, pe, Syntax::Production::COMMA_SEP );
        first = false;
    }
    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderModuleCtor( const Render::Kit &kit, TreePtr<Module> m ) try
{
    string s;
    
    s += "SC_CTOR( " + RenderIdentifier( kit, m->identifier ) + " )";
    int first = true;             
    auto sorted_members = sc.GetTreePtrOrdering(m->members);
    for( TreePtr<Node> pd : sorted_members )
    {
        // Bodge an init list that names any fields we have that are modules
        // and initialises any fields with initialisers
        if( TreePtr<Field> f = DynamicTreePtrCast<Field>(pd) )
        {
            if( TreePtr<TypeIdentifier> tid = DynamicTreePtrCast<TypeIdentifier>(f->type) ) try
			{
                if( TreePtr<Record> r = GetRecordDeclaration(kit, tid).GetTreePtr() )
                {
                    if( DynamicTreePtrCast<Module>(r) )
                    {
                        if( first )
                            s += " :";
                        else
                            s += ",";
                        string ids = RenderIdentifier(kit, f->identifier);                           
                        s += SYSTEMC_MARKER() + ids + "(\"" + ids + "\")";
                        first = false;
                    }   
				}
			}
			catch(DeclarationOf::DeclarationNotFound &)
			{
				// No decl so probably a system type. 
			}
        }
                    
        // Where data members are initialised, generate the init into the init list. We will 
        // inhibit rendering of these inits in the module decls. TODO inconsistent 
        // with normal C++ constructors where the inits should already be in the correct place.
        if( TreePtr<Field> i = DynamicTreePtrCast<Field>(pd) )
        {
            TRACE("Got ")(*i)(" init is ")(*(i->initialiser))(" %d %d\n", 
                    (int)(bool)DynamicTreePtrCast<Callable>(i->type),
                    (int)(bool)DynamicTreePtrCast<Uninitialised>(i->initialiser) );                   
        
            if( !DynamicTreePtrCast<Callable>(i->type) && !DynamicTreePtrCast<Uninitialised>(i->initialiser) )
            {                   
                if( first )
                    s += " :";
                else
                    s += ",";
                string ids = RenderIdentifier(kit, i->identifier);                           
                string inits = RenderExpression(kit, i->initialiser, Syntax::Production::COMMA_SEP);
                s += SYSTEMC_MARKER() + ids;
                if( DynamicTreePtrCast< MakeArray >(i->initialiser) )
					s += inits; // MakeArray as an init should render brace-initialiser syntax
				else
					s += "(" + inits + ")";
                first = false;                 
            }
        }                      
    }    
    s += SYSTEMC_MARKER() + "{\n";
    for( TreePtr<Node> pd : sorted_members )
        if( TreePtr<Field> f = DynamicTreePtrCast<Field>(pd) )
            if( TreePtr<Process> r = DynamicTreePtrCast<Process>(f->type) )
                s += r->GetToken() + "(" + RenderIdentifier( kit, f->identifier ) + ");" + SYSTEMC_MARKER();
    s += "}" + SYSTEMC_MARKER();
    
    return s;
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
        return RenderAccess( kit, this_access ) + ":\n";
    }
    
    return "";	
}								  


string Render::RenderScope( const Render::Kit &kit, 
							TreePtr<Scope> key,
							TreePtr<AccessSpec> init_access ) try
{
    TRACE();

    Sequence<Declaration> sorted = SortDecls( key->members, true, unique_ids );
    backing_ordering[key] = sorted;

    // Emit an incomplete for each record
    string s;
    for( TreePtr<Declaration> pd : sorted ) //for( int i=0; i<sorted.size(); i++ )
        if( TreePtr<Record> r = DynamicTreePtrCast<Record>(pd) ) // is a record
            if( !DynamicTreePtrCast<Enum>(r) ) // but not an enum
            {
				if( init_access )
					s += MaybeRenderFieldAccess( kit, r, &init_access );
                s += RenderRecordProto( kit, r ) + ";\n";
			}

    // For SystemC modules, we generate a constructor based on the other decls in
    // the module. Nothing goes in the Inferno tree for a module constructor, since
    // it is an elaboration mechanism, not funcitonal.
    TreePtr<Module> sc_module = DynamicTreePtrCast<Module>(key);
    if( sc_module )
    {
 		if( init_access )
			s += MaybeRenderAccessColon( kit, MakeTreeNode<Public>(), &init_access );		
        s += RenderModuleCtor( kit, sc_module ) + SYSTEMC_MARKER();
	}
	
    // Emit the actual declarations, sorted for dependencies
    for( TreePtr<Declaration> d : sorted )
    {
		if( init_access )
			s += MaybeRenderFieldAccess( kit, d, &init_access );		
        s += RenderDeclaration( kit, d );
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
		
    backing_ordering[key] = sorted;
		    
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
		string name = RenderIdentifier(kit, o->identifier);
        s += RenderType( kit, o->type, name, Syntax::Production::IDENTIFIER, false );
    		
        first = false;
    }
    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderMismatchException( string fname, const Mismatch &me )
{
    return "«"+fname+"() error: "+me.What()+"»";
}

