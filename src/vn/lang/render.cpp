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
    string( "»" );

#define ERROR_UNSUPPORTED(P) \
    ERROR_UNKNOWN( P ? typeid(*P).name() : "<nullptr>" );

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

// Note: this does not modify the program tree, and that can be checked by 
// defining TEST_FOR_UNMODIFIED_TREE and retesting everything.
//#define TEST_FOR_UNMODIFIED_TREE
string Render::RenderToString( TreePtr<Node> root )
{   
    DefaultTransUtils utils(root);
    TransKit kit { &utils };
    
#ifdef TEST_FOR_UNMODIFIED_TREE    
    temp_old_root = dynamic_pointer_cast<Program>(root);
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
	// it that it's doing a pre-pass so it can adapt it's behaviour.
    RenderScope( kit, root_scope ); 
    deferred_decls.clear();

    // Make the identifiers unique
    unique.clear();
    unique.UniquifyScope( root_scope );

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


bool Render::IsSystemC( const TransKit &, TreePtr<Node> root )
{ 
    Walk e(root, nullptr, nullptr);
    for( const TreePtrInterface &n : e )
        if( dynamic_pointer_cast<SCConstruct>((TreePtr<Node>)n) )
            return true;
    return false;
}


string Render::RenderLiteral( const TransKit &kit, TreePtr<Literal> sp ) try
{
	(void)kit;
    return Sanitise( sp->GetRender() );
}
DEFAULT_CATCH_CLAUSE


string Render::RenderIdentifier( const TransKit &kit, TreePtr<Identifier> id ) try
{
	(void)kit;
    string ids;
    if( id )
    {
        if( TreePtr<SpecificIdentifier> ii = DynamicTreePtrCast<SpecificIdentifier>( id ) )
        {
            if( unique.count(ii) == 0 )
                return ERROR_UNKNOWN( SSPrintf("identifier %s undeclared", ii->GetRender().c_str() ) );
            ids = unique.at(ii);
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


string Render::RenderScopePrefix( const TransKit &kit, TreePtr<Identifier> id ) try
{
    TreePtr<Node> scope = GetScope( root_scope, id );
        
    //TRACE("%p %p %p\n", program.get(), scope.get(), scope_stack.top().get() );
    if( scope == scope_stack.top() )
        return string(); // local scope
    else if( scope == root_scope )
        return " ::";
    else if( TreePtr<Enum> e = DynamicTreePtrCast<Enum>( scope ) ) // <- for enum
        return RenderScopePrefix( kit, e->identifier );    // omit scope for the enum itself
    else if( TreePtr<Record> r = DynamicTreePtrCast<Record>( scope ) ) // <- for class, struct, union
        return RenderScopedIdentifier( kit, r->identifier ) + "::";
    else if( DynamicTreePtrCast<CallableParams>( scope ) ||  // <- this is for params
             DynamicTreePtrCast<Compound>( scope ) ||    // <- this is for locals in body
             DynamicTreePtrCast<StatementExpression>( scope ) )    // <- this is for locals in body
        return string();
    else
        return ERROR_UNSUPPORTED( scope );
}
DEFAULT_CATCH_CLAUSE


string Render::RenderScopedIdentifier( const TransKit &kit, TreePtr<Identifier> id, bool bracketize_cpp_scope ) try
{
    string s_scope = RenderScopePrefix( kit, id );
    string s = s_scope + RenderIdentifier( kit, id );
    if( bracketize_cpp_scope && !s_scope.empty() )
		s = "(" + s + ")"; // Bracketise if there's anything in the C++ scope specifier
    TRACE("Render scoped identifier %s\n", s.c_str() );
    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderIntegralType( const TransKit &kit, TreePtr<Integral> type, string object ) try
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


string Render::RenderFloatingType( const TransKit &kit, TreePtr<Floating> type ) try
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


string Render::RenderType( const TransKit &kit, TreePtr<Type> type, string object, bool constant ) try
{
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
        return RenderType( kit, f->return_type, "(" + object + ")(" + RenderParams(kit, f) + ")" + const_str );
    else if( TreePtr<Process> f = DynamicTreePtrCast< Process >(type) )
        return "void " + object + "()" + const_str;
    else if( TreePtr<Pointer> p = DynamicTreePtrCast< Pointer >(type) )
        return RenderType( kit, p->destination, const_str + "(*" + object + ")", false ); // TODO Pointer node to indicate constancy of pointed-to object - would go into this call to RenderType
    else if( TreePtr<Reference> r = DynamicTreePtrCast< Reference >(type) )
        return RenderType( kit, r->destination, const_str + "(&" + object + ")" );
    else if( TreePtr<Array> a = DynamicTreePtrCast< Array >(type) )
        return RenderType( kit, 
                           a->element, 
                           object.empty() ? 
                               "[" + RenderExpression(kit, a->size) + "]" : 
                               "(" + object + "[" + RenderExpression(kit, a->size) + "])", 
                           constant );
    else if( TreePtr<Typedef> t = DynamicTreePtrCast< Typedef >(type) )
        return const_str + RenderIdentifier(kit, t->identifier) + sobject;
    else if( TreePtr<SpecificTypeIdentifier> ti = DynamicTreePtrCast< SpecificTypeIdentifier >(type) )
        return const_str + RenderScopedIdentifier(kit, ti) + sobject;
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


string Render::RenderOperator( const TransKit &kit, TreePtr<Operator> op, Sequence<Expression> &operands ) try
{
    ASSERT(op);
    string s;
    Sequence<Expression>::iterator operands_it = operands.begin();
    if( DynamicTreePtrCast< MakeArray >(op) )
    {
        s = "{ " + RenderOperandSequence( kit, operands, ", " ) + " }";
    }
    else if( DynamicTreePtrCast< ConditionalOperator >(op) )
    {
        s = RenderExpression( kit, *operands_it, true ) + " ? ";
        ++operands_it;
        s += RenderExpression( kit, *operands_it, true ) + " : ";
        ++operands_it;
        s += RenderExpression( kit, *operands_it, true );           
    }
    else if( DynamicTreePtrCast< Subscript >(op) )
    {
        s = RenderExpression( kit, *operands_it, true ) + "[";
        ++operands_it;
        s += RenderExpression( kit, *operands_it, false ) + "]";
    }
#define INFIX(TOK, TEXT, NODE_SHAPED, BASE, CAT) \
    else if( DynamicTreePtrCast<NODE_SHAPED>(op) ) \
    { \
        s = RenderExpression( kit, *operands_it, true ); \
        s += TEXT; \
        ++operands_it; \
        s += RenderExpression( kit, *operands_it, true ); \
    }
#define PREFIX(TOK, TEXT, NODE_SHAPED, BASE, CAT) \
    else if( DynamicTreePtrCast<NODE_SHAPED>(op) ) \
    { \
        s = TEXT; \
        s += RenderExpression( kit, *operands_it, true, \
                               !!TreePtr<AddressOf>::DynamicCast(op) ); /* Prevent interpretation as a member function pointer literal */ \
    }
#define POSTFIX(TOK, TEXT, NODE_SHAPED, BASE, CAT) \
    else if( DynamicTreePtrCast<NODE_SHAPED>(op) ) \
    { \
        s = RenderExpression( kit, *operands_it, true ); \
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


string Render::RenderCall( const TransKit &kit, TreePtr<Call> call ) try
{
    string s;

    // Render the expression that resolves to the function name unless this is
    // a constructor call in which case just the name of the thing being constructed.
    if( TreePtr<Expression> base = TypeOf::instance.TryGetConstructedExpression( kit, call ).GetTreePtr() )
        s += RenderExpression( kit, base, true );
    else
        s += RenderExpression( kit, call->callee, true );

    s += "(";

    // If CallableParams, generate some arguments, resolving the order using the original function type
    TreePtr<Node> ctype = TypeOf::instance(call->callee, root_scope).GetTreePtr();
    ASSERT( ctype );
    if( TreePtr<CallableParams> cp = DynamicTreePtrCast<CallableParams>(ctype) )
        s += RenderMapInOrder( kit, call, cp, ", ", false );

    s += ")";
    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderExpression( const TransKit &kit, TreePtr<Initialiser> expression, bool bracketize, bool bracketize_cpp_scope ) try
{
    //TRACE("%p\n", expression.get());
    string before = bracketize ? "(" : "";
    string after = bracketize ? ")" : "";

    if( DynamicTreePtrCast< Uninitialised >(expression) )
        return string();
    else if( TreePtr<StatementExpression> ce = DynamicTreePtrCast< StatementExpression >(expression) )
    {
        AutoPush< TreePtr<Node> > cs( scope_stack, ce );
        string s = "({ ";
        s += RenderScope( kit, ce ); // Must do this first to populate backing list
        s += RenderStatementSequence( kit, ce->statements );
        return s + "})";
    }
    else if( TreePtr<SpecificLabelIdentifier> li = DynamicTreePtrCast< SpecificLabelIdentifier >(expression) )
        return before +
               "&&" + RenderIdentifier( kit, li ) + // label-as-variable (GCC extension)
               after;
    else if( TreePtr<InstanceIdentifier> ii = DynamicTreePtrCast< InstanceIdentifier >(expression) )
        return RenderScopedIdentifier( kit, ii, bracketize_cpp_scope );
    else if( TreePtr<SizeOf> pot = DynamicTreePtrCast< SizeOf >(expression) )
        return before +
               "sizeof(" + RenderType( kit, pot->operand, "" ) + ")" +
               after;
    else if( TreePtr<AlignOf> pot = DynamicTreePtrCast< AlignOf >(expression) )
        return before +
               "alignof(" + RenderType( kit, pot->operand, "" ) + ")" +
               after;
    else if( TreePtr<NonCommutativeOperator> nco = DynamicTreePtrCast< NonCommutativeOperator >(expression) )
        return before +
               RenderOperator( kit, nco, nco->operands ) +
               after;
    else if( TreePtr<CommutativeOperator> co = DynamicTreePtrCast< CommutativeOperator >(expression) )
    {
        Sequence<Expression> seq_operands;
        // Operands are in collection, so sort them and put them in a sequence
        for( TreePtr<Node> o : sc.GetTreePtrOrdering(co->operands) )
            seq_operands.push_back( TreePtr<Expression>::DynamicCast(o) );
        return before +
               RenderOperator( kit, co, seq_operands ) +
               after;
    }
    else if( TreePtr<Call> c = DynamicTreePtrCast< Call >(expression) )
        return before +
               RenderCall( kit, c ) +
               after;
    else if( TreePtr<New> n = DynamicTreePtrCast< New >(expression) )
        return before +
               (DynamicTreePtrCast<Global>(n->global) ? "::" : "") +
               "new(" + RenderOperandSequence( kit, n->placement_arguments, ", " ) + ") " +
               RenderType( kit, n->type, "" ) +
               (n->constructor_arguments.empty() ? "" : "(" + RenderOperandSequence( kit, n->constructor_arguments, ", " ) + ")" ) +
               after;
    else if( TreePtr<Delete> d = DynamicTreePtrCast< Delete >(expression) )
        return before +
               (DynamicTreePtrCast<Global>(d->global) ? "::" : "") +
               "delete" +
               (DynamicTreePtrCast<DeleteArray>(d->array) ? "[]" : "") +
               " " + RenderExpression( kit, d->pointer, true ) +
               after;
    else if( TreePtr<Lookup> a = DynamicTreePtrCast< Lookup >(expression) )
        return before +
               RenderExpression( kit, a->base, true ) + "." +
               RenderScopedIdentifier( kit, a->member ) +
               after;
    else if( TreePtr<Cast> c = DynamicTreePtrCast< Cast >(expression) )
        return before +
               "(" + RenderType( kit, c->type, "" ) + ")" +
               RenderExpression( kit, c->operand, false ) +
               after;
    else if( TreePtr<MakeRecord> ro = DynamicTreePtrCast< MakeRecord >(expression) )
        return before +
               RenderMakeRecord( kit, ro ) +
               after;
    else if( TreePtr<Literal> l = DynamicTreePtrCast< Literal >(expression) )
        return before +
               RenderLiteral( kit, l ) +
               after;
    else if( DynamicTreePtrCast< This >(expression) )
        return before +
               "this" +
               after;
    else if( TreePtr<DeltaCount> dc = DynamicTreePtrCast<DeltaCount>(expression) ) 
        return before +
               dc->GetToken() + "()" +
               after;
    else
        return ERROR_UNSUPPORTED(expression);
}
DEFAULT_CATCH_CLAUSE


string Render::RenderMakeRecord( const TransKit &kit, TreePtr<MakeRecord> ro ) try
{
    string s;

    // Get the record
    TreePtr<TypeIdentifier> id = DynamicTreePtrCast<TypeIdentifier>(ro->type);
    ASSERT(id);

    TreePtr<Record> r = GetRecordDeclaration(kit, id).GetTreePtr();

    s += "(";
    s += RenderType( kit, ro->type, "" );
    s += "){ ";
    s += RenderMapInOrder( kit, ro, r, ", ", false );
    s += " }";
    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderMapInOrder( const TransKit &kit, 
                                 TreePtr<MapOperator> ro,
                                 TreePtr<Node> key,
                                 string separator,
                                 bool separate_last ) try
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
                            s += separator;
                        s += RenderExpression( kit, mi->value );
                        first = false;
                    }
                }
            }
        }
        if( separate_last )
            s += separator;
    }
    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderAccess( const TransKit &kit, TreePtr<AccessSpec> current_access ) try
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


string Render::RenderStorage( const TransKit &kit, TreePtr<Instance> st ) try
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


void Render::ExtractInits( const TransKit &kit, Sequence<Statement> &body, Sequence<Statement> &inits, Sequence<Statement> &remainder )
{
    for( TreePtr<Statement> s : body )
    {
        if( TreePtr<Call> o = DynamicTreePtrCast< Call >(s) )
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


string Render::RenderInstance( const TransKit &kit, TreePtr<Instance> o, 
                               bool show_storage, bool show_type, bool show_scope_res, bool show_init, string sep ) try
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

    if( show_storage )
    {
        s += RenderStorage(kit, o);
    }

    if( show_scope_res )
        name += RenderScopePrefix(kit, o->identifier);

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

    if( show_type )
        s += RenderType( kit, o->type, name, constant );
    else
        s = name;

    bool callable = (bool)DynamicTreePtrCast<Callable>(o->type);

    // If object is really a module, bodge in a name as a constructor parameter
    // But not for fields - they need an init list, done in RenderScope()
    if( !DynamicTreePtrCast<Field>(o) )
        if( TreePtr<TypeIdentifier> tid = DynamicTreePtrCast<TypeIdentifier>(o->type) )
            if( TreePtr<Record> r = GetRecordDeclaration(kit, tid).GetTreePtr() )
                if( DynamicTreePtrCast<Module>(r) )
                {
                    s += "(\"" + RenderIdentifier(kit, o->identifier) + "\")" + sep;
                    return s;
                }
    
    if( !show_init || DynamicTreePtrCast<Uninitialised>(o->initialiser) )
    {
        // Don't render any initialiser
        s += sep;
    }
    else if( callable )
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
            s += " : ";
            s += RenderConstructorInitList( kit, inits );
        }

        // Render the other stuff as a Compound so we always get {} in all cases
        auto r = MakeTreeNode<Compound>();
        r->members = members;
        r->statements = remainder;
        s += "\n" + RenderStatement(kit, r, "");
        // Surround functions with blank lines
        s = '\n' + s + '\n';
    }
    else
    {
        if( TreePtr<Expression> ei = DynamicTreePtrCast<Expression>( o->initialiser ) )
        {
            // Render expression with an assignment
            AutoPush< TreePtr<Node> > cs( scope_stack, GetScope( root_scope, o->identifier ) );
            s += " = " + RenderExpression(kit, ei) + sep;
        }
        else
        {
            s += ERROR_UNSUPPORTED(o->initialiser);
        }
    }

    return s;
}
DEFAULT_CATCH_CLAUSE


// Non-const static objects in records and functions 
// get split into a part that goes into the record (main line of rendering) and
// a part that goes separately (deferred_decls gets appended at the very end).
// Do all functions, since SortDecls() ignores function bodies for dep analysis
bool Render::ShouldSplitInstance( const TransKit &kit, TreePtr<Instance> o ) 
{
    bool isfunc = !!DynamicTreePtrCast<Callable>( o->type );
    bool isnumber = !!DynamicTreePtrCast<Numeric>( o->type );

    if( TreePtr<TypeIdentifier> ti = DynamicTreePtrCast<TypeIdentifier>(o->type) )
        if( DynamicTreePtrCast<Enum>( GetRecordDeclaration(kit, ti).GetTreePtr() ) )
            isnumber = 1; // enum is like a number        

    bool split_var = false;
    if( TreePtr<Static> s = DynamicTreePtrCast<Static>(o) )
        if( DynamicTreePtrCast<NonConst>(s->constancy) || !isnumber )
            split_var = true;

    return ( DynamicTreePtrCast<Record>( scope_stack.top() ) &&
               split_var ) || 
               isfunc;
}


string Render::RenderDeclaration( const TransKit &kit, TreePtr<Declaration> declaration,
                                  string sep, TreePtr<AccessSpec> *current_access,
                                  bool force_incomplete ) try
{
    TRACE();
    string s;

    TreePtr<AccessSpec> this_access = MakeTreeNode<Public>();

    // Decide access spec for this declaration (explicit if instance, otherwise force to Public)
    if( TreePtr<Field> f = DynamicTreePtrCast<Field>(declaration) )
        this_access = f->access;

    // Now decide whether we actually need to render an access spec (ie has it changed?)
    if( current_access && // nullptr means dont ever render access specs
        typeid(*this_access) != typeid(**current_access) ) // current_access spec must have changed
    {
        s += RenderAccess( kit, this_access ) + ":\n";
        *current_access = this_access;
    }

    if( TreePtr<Instance> o = DynamicTreePtrCast<Instance>(declaration) )
    {
        if( ShouldSplitInstance(kit, o) )
        {
            s += RenderInstance( kit, o, true, true, false, false, sep );
            {
                AutoPush< TreePtr<Node> > cs( scope_stack, root_scope );
                deferred_decls += string("\n") + RenderInstance( kit, o, false, true, true, true, sep );
            }
        }
        else
        {
            // Otherwise, render everything directly using the default settings
            s += RenderInstance( kit, o, true, true, false, true, sep );
            //ASSERT( shownonfuncinit || DynamicTreePtrCast<Uninitialised>(o->initialiser) )("Instance: ")(o)("\nInitialiser: ")(o->initialiser)("\nScopes:\n")(scope_stack)("\nlooks like:\n")(s);
        }
    }
    else if( TreePtr<Typedef> t = DynamicTreePtrCast< Typedef >(declaration) )
    {
        s += "typedef " + RenderType( kit, t->type, RenderIdentifier(kit, t->identifier) ) + sep;
    }
    else if( TreePtr<Record> r = DynamicTreePtrCast< Record >(declaration) )
    {
        TreePtr<AccessSpec> a;
        shared_ptr<SCNamedRecord> scr = dynamic_pointer_cast< SCNamedRecord >(r);
        if( DynamicTreePtrCast< Class >(r) || scr )
        {
            s += "class";
            a = MakeTreeNode<Private>();
        }
        else if( DynamicTreePtrCast< Struct >(r) )
        {
            s += "struct";
            a = MakeTreeNode<Public>();
        }
        else if( DynamicTreePtrCast< Union >(r) )
        {
            s += "union";
            a = MakeTreeNode<Public>();
        }
        else if( DynamicTreePtrCast< Enum >(r) )
        {
            s += "enum";
            a = MakeTreeNode<Public>();
        }
        else
            return ERROR_UNSUPPORTED(declaration);

        // Name of the record
        s += " " + RenderIdentifier(kit, r->identifier);

        if( !force_incomplete )
        {
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
                        s += "public " + scr->GetToken();
                    }
                    for( TreePtr<Node> bn : sc.GetTreePtrOrdering(ir->bases) )
                    {
                        if( !first )
                            s += ", ";
                        first=false;
                        auto b = TreePtr<Base>::DynamicCast(bn);
                        ASSERT( b );
                        s += RenderAccess(kit, b->access) + " " + RenderIdentifier(kit, b->record);
                    }
                }
            }

            // Contents
            AutoPush< TreePtr<Node> > cs( scope_stack, r );
            s += "\n{\n";
            if( DynamicTreePtrCast< Enum >(r) )
				s += RenderEnumBody( kit, r->members );
			else
				s += RenderScope( kit, r, a );			
            s += "}";                                 
        }

        s += ";\n";
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


string Render::RenderStatement( const TransKit &kit, TreePtr<Statement> statement, string sep ) try
{
    TRACE();
    if( !statement )
        return sep;
    //printf( "%s %d things\n", typeid(*statement).name(), statement->Itemise().size() );
    if( TreePtr<Declaration> d = DynamicTreePtrCast< Declaration >(statement) )
        return RenderDeclaration( kit, d, sep );
    else if( TreePtr<Compound> c = DynamicTreePtrCast< Compound >(statement) )
    {
        AutoPush< TreePtr<Node> > cs( scope_stack, c );
        string s = "{\n";
        s += RenderScope( kit, c ); // Must do this first to populate backing list
        s += RenderStatementSequence( kit, c->statements );
        return s + "}\n";
    }
    else if( TreePtr<Expression> e = DynamicTreePtrCast< Expression >(statement) )
        return RenderExpression(kit, e) + sep;
    else if( TreePtr<Return> es = DynamicTreePtrCast<Return>(statement) )
        return "return " + RenderExpression(kit, es->return_value) + sep;
    else if( TreePtr<Goto> g = DynamicTreePtrCast<Goto>(statement) )
    {
        if( TreePtr<SpecificLabelIdentifier> li = DynamicTreePtrCast< SpecificLabelIdentifier >(g->destination) )
            return "goto " + RenderIdentifier(kit, li) + sep;  // regular goto
        else
            return "goto *(" + RenderExpression(kit, g->destination) + ")" + sep; // goto-a-variable (GCC extension)
    }
    else if( TreePtr<If> i = DynamicTreePtrCast<If>(statement) )
    {
		bool else_clause = !DynamicTreePtrCast<Nop>(i->else_body); // Nop means no else clause
        string s;
        s += "if( " + RenderExpression(kit, i->condition) + " )\n";
        bool sub_if = !!DynamicTreePtrCast<If>(i->body);
        if( sub_if && else_clause )
             s += "{\n"; // Note: braces there to clarify else binding eg if(a) if(b) foo; else how_do_i_bind;
        s += RenderStatement(kit, i->body, ";\n");
        if( sub_if && else_clause )
             s += "}\n";
        if( else_clause )  
            s += "else\n" +
                 RenderStatement(kit, i->else_body, ";\n");
        return s;
    }
    else if( TreePtr<While> w = DynamicTreePtrCast<While>(statement) )
        return "while( " + RenderExpression(kit, w->condition) + " )\n" +
               RenderStatement(kit, w->body, ";\n");
    else if( TreePtr<Do> d = DynamicTreePtrCast<Do>(statement) )
        return "do\n" +
               RenderStatement(kit, d->body, ";\n") +
               "while( " + RenderExpression(kit, d->condition) + " )" + sep;
    else if( TreePtr<For> f = DynamicTreePtrCast<For>(statement) )
        return "for( " + RenderStatement(kit, f->initialisation, "") + "; " + RenderExpression(kit, f->condition) + "; "+ RenderStatement(kit, f->increment, "") + " )\n" +
               RenderStatement(kit, f->body, ";\n");
    else if( TreePtr<Switch> s = DynamicTreePtrCast<Switch>(statement) )
        return "switch( " + RenderExpression(kit, s->condition) + " )\n" +
               RenderStatement(kit, s->body, ";\n");
    else if( TreePtr<Case> c = DynamicTreePtrCast<Case>(statement) )
        return "case " + RenderExpression(kit, c->value) + ":;\n";
    else if( TreePtr<RangeCase> rc = DynamicTreePtrCast<RangeCase>(statement) )
        return "case " + RenderExpression(kit, rc->value_lo) + " ... " + RenderExpression(kit, rc->value_hi) + ":\n";
    else if( DynamicTreePtrCast<Default>(statement) )
        return "default:;\n";
    else if( DynamicTreePtrCast<Continue>(statement) )
        return "continue" + sep;
    else if( DynamicTreePtrCast<Break>(statement) )
        return "break" + sep;
    else if( DynamicTreePtrCast<Nop>(statement) )
        return sep;
    else if( TreePtr<WaitDynamic> c = DynamicTreePtrCast<WaitDynamic>(statement) ) 
        return c->GetToken() + "( " + RenderExpression(kit, c->event) + " );\n";
    else if( TreePtr<WaitStatic> c = DynamicTreePtrCast<WaitStatic>(statement) ) 
        return c->GetToken() + "();\n";
    else if( TreePtr<WaitDelta> c = DynamicTreePtrCast<WaitDelta>(statement) )
        return c->GetToken() + "(SC_ZERO_TIME);\n";
    else if( TreePtr<NextTriggerDynamic> c = DynamicTreePtrCast<NextTriggerDynamic>(statement) ) 
        return c->GetToken() + "( " + RenderExpression(kit, c->event) + " );\n";
    else if( TreePtr<NextTriggerStatic> c = DynamicTreePtrCast<NextTriggerStatic>(statement) ) 
        return c->GetToken() + "();\n";
    else if( TreePtr<NextTriggerDelta> c = DynamicTreePtrCast<NextTriggerDelta>(statement) ) 
        return c->GetToken() + "(SC_ZERO_TIME);\n";
    else if( TreePtr<TerminationFunction> tf = DynamicTreePtrCast<TerminationFunction>(statement) )
        return tf->GetToken() + "( " + RenderExpression(kit, tf->code) + " );\n";
    else if( TreePtr<NotifyImmediate> n = DynamicTreePtrCast<NotifyImmediate>(statement) )
        return RenderExpression( kit, n->event, true ) + "." + n->GetToken() + "();\n";
    else if( TreePtr<NotifyDelta> n = DynamicTreePtrCast<NotifyDelta>(statement) )
        return RenderExpression( kit, n->event, true ) + "." + n->GetToken() + "(SC_ZERO_TIME);\n";
    else
        return ERROR_UNSUPPORTED(statement);
}
DEFAULT_CATCH_CLAUSE


string Render::RenderStatementSequence( const TransKit &kit, 
                                        Sequence<CPPTree::Statement> spe ) try
{
    TRACE();
    string s;
    for( TreePtr<Statement> st : spe )    
        s += RenderStatement( kit, st, ";\n" );    
    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderDeclarationSequence( const TransKit &kit, 
                                          Sequence<Declaration> spe,
                                          TreePtr<AccessSpec> init_access ) try
{
    TRACE();
    string s;    
    for( TreePtr<Declaration> d : spe )
        s += RenderDeclaration( kit, d, ";\n", init_access ? &init_access : nullptr, false );
    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderConstructorInitList( const TransKit &kit, 
										  Sequence<Statement> spe ) try
{
    TRACE();
    string s; 
    bool first = true;
    for( TreePtr<Statement> st : spe )
    {
		if( !first )
			s += ",\n";
        if( auto e = TreePtr<Expression>::DynamicCast(st) )
            s += RenderExpression( kit, e );
        else 
            s += ERROR_UNSUPPORTED(st);
        first = false;
    }
    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderEnumBody( const TransKit &kit, 
                               Collection<Declaration> spe ) try
{
    TRACE();
    string s;
    for( TreePtr<Declaration> pe : spe )
    {
        if( auto o = TreePtr<Instance>::DynamicCast(pe) )
            s += RenderInstance( kit, o, false, false, false, true, ",\n" );
        else 
            s += ERROR_UNSUPPORTED(pe);
    }
    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderOperandSequence( const TransKit &kit, 
                                      Sequence<Expression> spe,
                                      string separator ) try
{
    TRACE();
    string s;
    bool first = true;
    for( TreePtr<Expression> pe : spe )
    {
		if( !first )
			s += separator;
        s += RenderExpression( kit, pe );
        first = false;
    }
    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderModuleCtor( const TransKit &kit, 
                                 TreePtr<Module> m,
                                 TreePtr<AccessSpec> *access ) try
{
    string s;
    
    // SystemC module, we must produce a constructor in SC style, do this as inline
    if( !DynamicTreePtrCast<Public>(*access) )
    {
        s += "public:\n";
        *access = MakeTreeNode<Public>();// note that we left the access as public
    }
    s += "SC_CTOR( " + RenderIdentifier( kit, m->identifier ) + " )";
    int first = true;             
    auto sorted_members = sc.GetTreePtrOrdering(m->members);
    for( TreePtr<Node> pd : sorted_members )
    {
        // Bodge an init list that names any fields we have that are modules
        // and initialises any fields with initialisers
        if( TreePtr<Field> f = DynamicTreePtrCast<Field>(pd) )
            if( TreePtr<TypeIdentifier> tid = DynamicTreePtrCast<TypeIdentifier>(f->type) )
                if( TreePtr<Record> r = GetRecordDeclaration(kit, tid).GetTreePtr() )
                    if( DynamicTreePtrCast<Module>(r) )
                    {
                        if( first )
                            s += " :";
                        else
                            s += ",";
                        string ids = RenderIdentifier(kit, f->identifier);                           
                        s += "\n" + ids + "(\"" + ids + "\")";
                        first = false;
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
                string inits = RenderExpression(kit, i->initialiser);
                s += "\n" + ids;
                if( DynamicTreePtrCast< MakeArray >(i->initialiser) )
					s += inits; // MakeArray as an init should render brace-initialiser syntax
				else
					s += "(" + inits + ")";
                first = false;                 
            }
        }                      
    }    
    s += "\n{\n";
    for( TreePtr<Node> pd : sorted_members )
        if( TreePtr<Field> f = DynamicTreePtrCast<Field>(pd) )
            if( TreePtr<Process> r = DynamicTreePtrCast<Process>(f->type) )
                s += r->GetToken() + "(" + RenderIdentifier( kit, f->identifier ) + ");\n";
    s += "}\n";
    
    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderScope( const TransKit &kit, 
							TreePtr<Scope> key,
							TreePtr<AccessSpec> init_access ) try
{
    TRACE();

    Sequence<Declaration> sorted = SortDecls( key->members, true, &unique );
    backing_ordering[key] = sorted;

    // Emit an incomplete for each record
    string s;
    for( TreePtr<Declaration> pd : sorted ) //for( int i=0; i<sorted.size(); i++ )
        if( TreePtr<Record> r = DynamicTreePtrCast<Record>(pd) ) // is a record
            if( !DynamicTreePtrCast<Enum>(r) ) // but not an enum
                s += RenderDeclaration( kit, r, ";\n", init_access ? &init_access : nullptr, true );

    // For SystemC modules, we generate a constructor based on the other decls in
    // the module. Nothing goes in the Inferno tree for a module constructor, since
    // it is an elaboration mechanism, not funcitonal.
    TreePtr<Module> sc_module = DynamicTreePtrCast<Module>(key);
    if( sc_module )
        s += RenderModuleCtor( kit, sc_module, &init_access );

    // Emit the actual declarations, sorted for dependencies
    s += RenderDeclarationSequence( kit, sorted, init_access );
    TRACE();
    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderParams( const TransKit &kit, 
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
        s += RenderDeclaration( kit, d, "", nullptr, false );
        first = false;
    }
    return s;
}
DEFAULT_CATCH_CLAUSE


string Render::RenderMismatchException( string fname, const Mismatch &me )
{
    return "«"+fname+"() error: "+me.What()+"»";
}

