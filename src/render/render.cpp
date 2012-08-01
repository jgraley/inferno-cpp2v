#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "helpers/transformation.hpp"
#include "tree/typeof.hpp"
#include "common/trace.hpp"
#include "common/read_args.hpp"
#include "tree/type_db.hpp"
#include "helpers/walk.hpp"
#include "tree/misc.hpp"
#include "tree/scope.hpp"
#include "sort_decls.hpp"
#include "render.hpp"
#include "clang/Parse/DeclSpec.h"
#include "uniquify_identifiers.hpp"

using namespace CPPTree;
using namespace SCTree;

// Don't like the layout of rendered code?
// Install "indent" on your UNIX box and pipe the output through iindent.sh

// TODO maybe reduce () by using a simplified scheme, ie divide operators into "high" and "low"
// categories and elide on low(high()). Note that a full scheme makes code unreadable if
// user does not have them memorised. Maybe just encode those priorities that are well known.

// TODO indent back to previous level at end of string
#define ERROR_UNKNOWN(V) \
    string( "\n#error " ) + \
    string( V ) + \
    string( " not supported in " ) + \
    string( INFERNO_CURRENT_FUNCTION ) + \
    string( "\n" );

#define ERROR_UNSUPPORTED(P) \
    ERROR_UNKNOWN( P ? typeid(*P).name() : "<NULL>" );


Render::Render( string of ) :
    outfile( of )
{
}


TreePtr<Node> Render::operator()( TreePtr<Node> context, TreePtr<Node> root )
{
	// Render can only work on a whole program
	ASSERT( context == root );
	program = dynamic_pointer_cast<Program>(root);
	ASSERT( program );
	AutoPush< TreePtr<Scope> > cs( scope_stack, program );
	unique.clear();
	unique.UniquifyScope( context );

    string s;

    if( IsSystemC( root ) )
        s += "#include \"isystemc.h\"\n\n";

	s += RenderDeclarationCollection( program, ";\n", true ); // gets the .hpp stuff directly

	s += deferred_decls; // these could go in a .cpp file

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
	program = TreePtr<Program>();
	return root; // no change
}


bool Render::IsSystemC( TreePtr<Node> root )
{ 
    Walk e(root);
    FOREACH( TreePtr<Node> n, e )
        if( dynamic_pointer_cast<SCConstruct>(n) )
            return true;
    return false;
}


string Render::RenderLiteral( TreePtr<Literal> sp )
{
	return Sanitise( *sp );
}


string Render::RenderIdentifier( TreePtr<Identifier> id )
{
	string ids;
	if( id )
	{
		if( TreePtr<SpecificIdentifier> ii = dynamic_pointer_cast<SpecificIdentifier>( id ) )
			ids = unique[ii];
		else
			ids = ERROR_UNSUPPORTED( (id) );

		TRACE( "%s\n", ids.c_str() );
	}
	else
	{
		TRACE();
	}
	return ids;
}


string Render::RenderScopePrefix( TreePtr<Identifier> id )
{
	TreePtr<Scope> scope = GetScope( program, id );
	TRACE("%p %p %p\n", program.get(), scope.get(), scope_stack.top().get() );
	if( scope == scope_stack.top() )
		return string(); // local scope
	else if( scope == program )
		return " ::";
	else if( TreePtr<Enum> e = dynamic_pointer_cast<Enum>( scope ) ) // <- for enum
		return RenderScopePrefix( e->identifier );    // omit scope for the enum itself
	else if( TreePtr<Record> r = dynamic_pointer_cast<Record>( scope ) ) // <- for class, struct, union
		return RenderScopedIdentifier( r->identifier ) + "::";
	else if( dynamic_pointer_cast<CallableParams>( scope ) ||  // <- this is for params
             dynamic_pointer_cast<Compound>( scope ) ||    // <- this is for locals in body
             dynamic_pointer_cast<CompoundExpression>( scope ) )    // <- this is for locals in body
		return string();
	else
		return ERROR_UNSUPPORTED( scope );
}


string Render::RenderScopedIdentifier( TreePtr<Identifier> id )
{
	string s = RenderScopePrefix( id ) + RenderIdentifier( id );
	TRACE("Render scoped identifier %s\n", s.c_str() );
	return s;
}


string Render::RenderIntegralType( TreePtr<Integral> type, string object )
{
	bool ds;
	unsigned width;
	TreePtr<SpecificInteger> ic = dynamic_pointer_cast<SpecificInteger>( type->width );
	ASSERT(ic)("width must be integer");
	width = ic->getLimitedValue();

	TRACE("width %d\n", width);

	if( width == TypeDb::char_bits )
		ds = TypeDb::char_default_signed;
	else
		ds = TypeDb::int_default_signed;

	// Produce signed or unsigned if required
	// Note: literal strings can be converted to char * but not unsigned char * or signed char *
	string s;
	if( dynamic_pointer_cast< Signed >(type) && !ds )
		s = "signed ";
	else if( dynamic_pointer_cast< Unsigned >(type) && ds )
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
	else    // unmatched defaults to int for bitfields
	{
		s += "int";
		bitfield = true;
	}

	s += " " + object;

	if( bitfield )
	{
	   char b[100];
	   sprintf(b, ":%d", width);
	   s += b;
	}

	return s;
}


string Render::RenderFloatingType( TreePtr<Floating> type )
{
	string s;
	TreePtr<SpecificFloatSemantics> sem = dynamic_pointer_cast<SpecificFloatSemantics>(type->semantics);
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


string Render::RenderType( TreePtr<Type> type, string object, bool constant )
{
	string sobject;
	if( !object.empty() )
		sobject = " " + object;
    
    string const_str = constant?"const ":"";

	TRACE();
	if( TreePtr<Integral> i = dynamic_pointer_cast< Integral >(type) )
		return const_str + RenderIntegralType( i, object );
	if( TreePtr<Floating> f = dynamic_pointer_cast< Floating >(type) )
		return const_str + RenderFloatingType( f ) + sobject;
	else if( dynamic_pointer_cast< Void >(type) )
		return const_str + "void" + sobject;
	else if( dynamic_pointer_cast< Boolean >(type) )
		return const_str + "bool" + sobject;
	else if( TreePtr<Constructor> c = dynamic_pointer_cast< Constructor >(type) )
		return object + "(" + RenderDeclarationCollection(c, ", ", false) + ")" + const_str;
	else if( TreePtr<Destructor> f = dynamic_pointer_cast< Destructor >(type) )
		return object + "()" + const_str;
	else if( TreePtr<Function> f = dynamic_pointer_cast< Function >(type) )
		return RenderType( f->return_type, "(" + object + ")(" + RenderDeclarationCollection(f, ", ", false) + ")" + const_str );
	else if( TreePtr<Process> f = dynamic_pointer_cast< Process >(type) )
		return "void " + object + "()" + const_str;
	else if( TreePtr<Pointer> p = dynamic_pointer_cast< Pointer >(type) )
		return RenderType( p->destination, const_str + "(*" + object + ")", false ); // TODO Pointer node to indicate constancy of pointed-to object - would go into this call to RenderType
	else if( TreePtr<Reference> r = dynamic_pointer_cast< Reference >(type) )
		return RenderType( r->destination, const_str + "(&" + object + ")" );
	else if( TreePtr<Array> a = dynamic_pointer_cast< Array >(type) )
		return RenderType( a->element, object.empty() ? "[" + RenderExpression(a->size) + "]" : "(" + object + "[" + RenderExpression(a->size) + "])", constant );
	else if( TreePtr<Typedef> t = dynamic_pointer_cast< Typedef >(type) )
		return const_str + RenderIdentifier(t->identifier) + sobject;
	else if( TreePtr<SpecificTypeIdentifier> ti = dynamic_pointer_cast< SpecificTypeIdentifier >(type) )
		return const_str + RenderScopedIdentifier(ti) + sobject;
	else if( shared_ptr<SCNamedIdentifier> sct = dynamic_pointer_cast< SCNamedIdentifier >(type) )
		return const_str + sct->GetToken() + sobject;
    else if( dynamic_pointer_cast<Labeley>(type) )
        return const_str + "void *" + object;
	else
		return ERROR_UNSUPPORTED(type);
}


// Insert escapes into a string so it can be put in source code
// TODO use \n \r etc and let printable ascii through
string Render::Sanitise( string s )
{
	string o;
	for( int i=0; i<s.size(); i++ )
	{
		char c[10];
		if( s[i] < ' ' )
			o += SSPrintf( c, "\\x%02x", s[i] );
		else
			o += s[i];
	}
	return o;
}


string Render::RenderOperator( TreePtr<Operator> op, Sequence<Expression> &operands )
{
	ASSERT(op);
	if( dynamic_pointer_cast< MakeArray >(op) )
		return "{ " + RenderOperandSequence( operands, ", ", false ) + " }";
	else if( dynamic_pointer_cast< Multiplexor >(op) )
		return RenderExpression( operands[0], true ) + " ? " +
			   RenderExpression( operands[1], true ) + " : " +
			   RenderExpression( operands[2], true );
	else if( dynamic_pointer_cast< Subscript >(op) )
		return RenderExpression( operands[0], true ) + "[" +
			   RenderExpression( operands[1], false ) + "]";
#define INFIX(TOK, TEXT, NODE, BASE, CAT) \
	else if( dynamic_pointer_cast<NODE>(op) ) \
		return RenderExpression( operands[0], true ) +\
			   TEXT +\
			   RenderExpression( operands[1], true );
#define PREFIX(TOK, TEXT, NODE, BASE, CAT) \
	else if( dynamic_pointer_cast<NODE>(op) ) \
		return TEXT +\
			   RenderExpression( operands[0], true );
#define POSTFIX(TOK, TEXT, NODE, BASE, CAT) \
	else if( dynamic_pointer_cast<NODE>(op) ) \
		return RenderExpression( operands[0], true ) +\
			   TEXT;
#include "tree/operator_db.inc"
	else
		return ERROR_UNSUPPORTED(op);
}


string Render::RenderCall( TreePtr<Call> call )
{
	string s;

	// Render the expression that resolves to the function name unless this is
	// a constructor call in which case just the name of the thing being constructed.
	if( TreePtr<Expression> base = TypeOf::instance.IsConstructorCall( program, call ) )
		s += RenderExpression( base, true );
	else
		s += RenderExpression( call->callee, true );

	s += "(";

	// If CallableParams, generate some arguments, resolving the order using the original function type
	TreePtr<Node> ctype = TypeOf::instance( program, call->callee );
	ASSERT( ctype );
	if( TreePtr<CallableParams> cp = dynamic_pointer_cast<CallableParams>(ctype) )
		s += RenderMapInOrder( call, cp, ", ", false );

	s += ")";
	return s;
}


string Render::RenderExpression( TreePtr<Initialiser> expression, bool bracketize_operator )
{
	TRACE("%p\n", expression.get());

	string before = bracketize_operator ? "(" : "";
	string after = bracketize_operator ? ")" : "";

	if( dynamic_pointer_cast< Uninitialised >(expression) )
		return string();
    else if( TreePtr<CompoundExpression> ce = dynamic_pointer_cast< CompoundExpression >(expression) )
    {
        AutoPush< TreePtr<Scope> > cs( scope_stack, ce );
        string s = "({ ";
        s += RenderDeclarationCollection( ce, "; ", true ); // Must do this first to populate backing list
        s += RenderSequence( ce->statements, "; ", true );
        return s + "})";
    }
	else if( TreePtr<SpecificLabelIdentifier> li = dynamic_pointer_cast< SpecificLabelIdentifier >(expression) )
		return before +
			   "&&" + RenderIdentifier( li ) + // label-as-variable (GCC extension)
			   after;
	else if( TreePtr<InstanceIdentifier> ii = dynamic_pointer_cast< InstanceIdentifier >(expression) )
		return RenderScopedIdentifier( ii );
	else if( TreePtr<SizeOf> pot = dynamic_pointer_cast< SizeOf >(expression) )
		return before +
			   "sizeof(" + RenderType( pot->operand, "" ) + ")" +
			   after;
	else if( TreePtr<AlignOf> pot = dynamic_pointer_cast< AlignOf >(expression) )
		return before +
			   "alignof(" + RenderType( pot->operand, "" ) + ")" +
			   after;
	else if( TreePtr<NonCommutativeOperator> nco = dynamic_pointer_cast< NonCommutativeOperator >(expression) )
		return before +
			   RenderOperator( nco, nco->operands ) +
			   after;
	else if( TreePtr<CommutativeOperator> co = dynamic_pointer_cast< CommutativeOperator >(expression) )
	{
		Sequence<Expression> seq_operands;
		// Operands are in collection, so move them to a container
		FOREACH( TreePtr<Expression> o, co->operands )
			seq_operands.push_back( o );
		return before +
			   RenderOperator( co, seq_operands ) +
			   after;
	}
	else if( TreePtr<Call> c = dynamic_pointer_cast< Call >(expression) )
		return before +
			   RenderCall( c ) +
			   after;
	else if( TreePtr<New> n = dynamic_pointer_cast< New >(expression) )
		return before +
			   (dynamic_pointer_cast<Global>(n->global) ? "::" : "") +
			   "new(" + RenderOperandSequence( n->placement_arguments, ", ", false ) + ") " +
			   RenderType( n->type, "" ) +
			   (n->constructor_arguments.empty() ? "" : "(" + RenderOperandSequence( n->constructor_arguments, ", ", false ) + ")" ) +
			   after;
	else if( TreePtr<Delete> d = dynamic_pointer_cast< Delete >(expression) )
		return before +
			   (dynamic_pointer_cast<Global>(d->global) ? "::" : "") +
			   "delete" +
			   (dynamic_pointer_cast<DeleteArray>(d->array) ? "[]" : "") +
			   " " + RenderExpression( d->pointer, true ) +
			   after;
	else if( TreePtr<Lookup> a = dynamic_pointer_cast< Lookup >(expression) )
		return before +
			   RenderExpression( a->base, true ) + "." +
			   RenderScopedIdentifier( a->member ) +
			   after;
	else if( TreePtr<Cast> c = dynamic_pointer_cast< Cast >(expression) )
		return before +
			   "(" + RenderType( c->type, "" ) + ")" +
			   RenderExpression( c->operand, false ) +
			   after;
	else if( TreePtr<MakeRecord> ro = dynamic_pointer_cast< MakeRecord >(expression) )
		return before +
			   RenderMakeRecord( ro ) +
			   after;
	else if( TreePtr<Literal> l = dynamic_pointer_cast< Literal >(expression) )
		return before +
			   RenderLiteral( l ) +
			   after;
	else if( dynamic_pointer_cast< This >(expression) )
		return before +
			   "this" +
			   after;
	else if( TreePtr<DeltaCount> dc = dynamic_pointer_cast<DeltaCount>(expression) ) 
	    return before +
	           dc->GetToken() + "()" +
	           after;
	else
		return ERROR_UNSUPPORTED(expression);
}


string Render::RenderMakeRecord( TreePtr<MakeRecord> ro )
{
	string s;

	// Get the record
	TreePtr<TypeIdentifier> id = dynamic_pointer_cast<TypeIdentifier>(ro->type);
	ASSERT(id);
	TreePtr<Record> r = GetRecordDeclaration(program, id);

	s += "(";
	s += RenderType( ro->type, "" );
	s += "){ ";
	s += RenderMapInOrder( ro, r, ", ", false );
	s += " }";
	return s;
}


string Render::RenderMapInOrder( TreePtr<MapOperator> ro,
						 TreePtr<Scope> r,
						 string separator,
						 bool separate_last )
{
	string s;

	// Get a reference to the ordered list of members for this record from a backing list
	if( !backing_ordering.IsExist( r ) )
	{
	    TRACE("Needed to see ")(*r)(" before ")(*ro)(" so map may not match; sorting now\n");	    
	    backing_ordering[r] = SortDecls( r->members, true );
	}
	ASSERT( backing_ordering.IsExist( r ) );
	Sequence<Declaration> &sd = backing_ordering[r];
	ASSERT( sd.size() == r->members.size() );
	bool first = true;
	FOREACH( TreePtr<Declaration> d, sd )
	{
		// We only care about instances...
		if( TreePtr<Instance> i = dynamic_pointer_cast<Instance>( d ) )
		{
			// ...and not function instances
			if( !dynamic_pointer_cast<Callable>( i->type ) )
			{
				// search init for matching member (TODO could avoid O(n^2) by exploiting the map)
				FOREACH( TreePtr<MapOperand> mi, ro->operands )
				{
					if( i->identifier == mi->identifier )
					{
						if( !first )
							s += separator;
						s += RenderExpression( mi->value );
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


string Render::RenderAccess( TreePtr<AccessSpec> current_access )
{
	if( dynamic_pointer_cast<Public>( current_access ) )
		return "public";
	else if( dynamic_pointer_cast<Private>( current_access ) )
		return "private";
	else if( dynamic_pointer_cast<Protected>( current_access ) )
		return "protected";
	else
		return ERROR_UNKNOWN("current_access spec");
}


string Render::RenderStorage( TreePtr<Instance> st )
{
	if( dynamic_pointer_cast<Program>( scope_stack.top() ) )
		return ""; // at top-level scope, everything is set to static, but don't actually output the word
	else if( dynamic_pointer_cast<Static>( st ) )
		return "static ";
	else if( dynamic_pointer_cast<Automatic>( st ) )
		return "auto ";
	else if( dynamic_pointer_cast<Temporary>( st ) )
		return "/*temp*/ ";
	else if( TreePtr<Field> no = dynamic_pointer_cast<Field>( st ) )
	{
		TreePtr<Virtuality> v = no->virt;
		if( dynamic_pointer_cast<Virtual>( v ) )
			return "virtual ";
		else if( dynamic_pointer_cast<NonVirtual>( v ) )
			return "";
		else
			return ERROR_UNKNOWN("virtualness");
	}
	else
		return ERROR_UNKNOWN("storage class");
}


void Render::ExtractInits( Sequence<Statement> &body, Sequence<Statement> &inits, Sequence<Statement> &remainder )
{
	FOREACH( TreePtr<Statement> s, body )
	{
		if( TreePtr<Call> o = dynamic_pointer_cast< Call >(s) )
		{
			if( TypeOf::instance.IsConstructorCall( program, o ) )
			{
				inits.push_back(s);
				continue;
			}
		}
		remainder.push_back(s);
	}
}


string Render::RenderInstance( TreePtr<Instance> o, string sep, bool showtype,
					   bool showstorage, bool showinit, bool showscope )
{
	string s;
    string name;
    bool constant=false;

	ASSERT(o->type);

	if( TreePtr<Static> st = dynamic_pointer_cast<Static>(o) )
		if( dynamic_pointer_cast<Const>(st->constancy) )
			constant = true;
	if( TreePtr<Field> f = dynamic_pointer_cast<Field>(o) )
		if( dynamic_pointer_cast<Const>(f->constancy) )
            constant = true;

	if( showstorage )
	{
		s += RenderStorage(o);
	}

	if( showscope )
		name += RenderScopePrefix(o->identifier);

	TreePtr<Constructor> con = dynamic_pointer_cast<Constructor>(o->type);
	TreePtr<Destructor> de = dynamic_pointer_cast<Destructor>(o->type);
	if( con || de )
	{
		TreePtr<Record> rec = dynamic_pointer_cast<Record>( GetScope( program, o->identifier ) );
		ASSERT( rec );        
		name += (de ? "~" : "");
		name += RenderIdentifier(rec->identifier);
	}
	else
	{
		name += RenderIdentifier(o->identifier);
	}

	if( showtype )
		s += RenderType( o->type, name, constant );
	else
		s = name;

	bool callable = dynamic_pointer_cast<Callable>(o->type);

    // If object is really a module, bodge in a name as a constructor parameter
    // But not for fields - they need an init list, done in RenderDeclarationCollection()
	if( !dynamic_pointer_cast<Field>(o) )
	    if( TreePtr<TypeIdentifier> tid = dynamic_pointer_cast<TypeIdentifier>(o->type) )
	        if( TreePtr<Record> r = GetRecordDeclaration(program, tid) )
	            if( dynamic_pointer_cast<Module>(r) )
	            {
	                s += "(\"" + RenderIdentifier(o->identifier) + "\")" + sep;
	                return s;
	            }
	
	if( !showinit || dynamic_pointer_cast<Uninitialised>(o->initialiser) )
	{
		// Don't render any initialiser
		s += sep;
	}
	else if( callable )
	{
		// Establish the scope of the function
		AutoPush< TreePtr<Scope> > cs( scope_stack, GetScope( program, o->identifier ) );

		// Put the contents of the body into a Compound-like form even if there's only one
		// Statement there - this is because we will wrangle with them later
		Sequence<Statement> code;
		Collection<Declaration> members;
		if( TreePtr<Compound> comp = dynamic_pointer_cast<Compound>(o->initialiser) )
		{
			members = comp->members;
			code = comp->statements;
		}
		else if( TreePtr<Statement> st = dynamic_pointer_cast<Statement>(o->initialiser) )
			code.push_back( st );
		else
			s += ERROR_UNSUPPORTED(o->initialiser);

		// Seperate the statements into constructor initialisers and "other stuff"
		Sequence<Statement> inits;
		Sequence<Statement> remainder;
		ExtractInits( code, inits, remainder );

		// Render the constructor initialisers if there are any
		if( !inits.empty() )
		{
			s += " : ";
			s += RenderSequence( inits, ", ", false, TreePtr<Public>(), true );
		}

		// Render the other stuff as a Compound so we always get {} in all cases
		TreePtr<Compound> r( new Compound );
		r->members = members;
		r->statements = remainder;
		s += "\n" + RenderStatement(r, "");
    }
	else
	{
		if( TreePtr<Expression> ei = dynamic_pointer_cast<Expression>( o->initialiser ) )
		{
			// Render expression with an assignment
			AutoPush< TreePtr<Scope> > cs( scope_stack, GetScope( program, o->identifier ) );
			s += " = " + RenderExpression(ei) + sep;
		}
		else
		{
			s += ERROR_UNSUPPORTED(o->initialiser);
		}
	}

	return s;
}


// Non-const static objects in records and functions 
// get split into a part that goes into the record (main line of rendering) and
// a part that goes separately (deferred_decls gets appended at the very end).
// Do all functions, since SortDecls() ignores function bodies for dep analysis
bool Render::ShouldSplitInstance( TreePtr<Instance> o )
{
	bool isfunc = !!dynamic_pointer_cast<Callable>( o->type );
	bool isnumber = !!dynamic_pointer_cast<Numeric>( o->type );
    if( TreePtr<TypeIdentifier> ti = dynamic_pointer_cast<TypeIdentifier>(o->type) )
        if( dynamic_pointer_cast<Enum>( GetRecordDeclaration(program, ti) ) )
            isnumber = 1; // enum is like a number        
	bool split_var = false;
	if( TreePtr<Static> s = dynamic_pointer_cast<Static>(o) )
		if( dynamic_pointer_cast<NonConst>(s->constancy) || !isnumber )
			split_var = true;
	return ( dynamic_pointer_cast<Record>( scope_stack.top() ) &&
			   split_var ) || 
			   isfunc;
}


string Render::RenderDeclaration( TreePtr<Declaration> declaration,
							 string sep, TreePtr<AccessSpec> *current_access,
						  bool showtype, bool force_incomplete )
{
	TRACE();
	string s;

	TreePtr<AccessSpec> this_access;

	// Decide access spec for this declaration (explicit if instance, otherwise force to Public)
	if( TreePtr<Field> f = dynamic_pointer_cast<Field>(declaration) )
		this_access = f->access;
	else
		this_access = MakeTreePtr<Public>();

	// Now decide whether we actually need to render an access spec (ie has it changed?)
	if( current_access && // NULL means dont ever render access specs
		typeid(*this_access) != typeid(**current_access) ) // current_access spec must have changed
	{
		s += RenderAccess( this_access ) + ":\n";
		*current_access = this_access;
	}

	if( TreePtr<Instance> o = dynamic_pointer_cast<Instance>(declaration) )
	{
		if( ShouldSplitInstance(o) )
		{
			s += RenderInstance( o, sep, showtype, showtype, false, false );
			{
				AutoPush< TreePtr<Scope> > cs( scope_stack, program );
				deferred_decls += string("\n") + RenderInstance( o, sep, showtype, false, true, true );
			}
		}
		else
		{
			// Otherwise, render everything directly using the default settings
			s += RenderInstance( o, sep, showtype, showtype, true, false );
		}
	}
	else if( TreePtr<Typedef> t = dynamic_pointer_cast< Typedef >(declaration) )
	{
		s += "typedef " + RenderType( t->type, RenderIdentifier(t->identifier) ) + sep;
	}
	else if( TreePtr<Record> r = dynamic_pointer_cast< Record >(declaration) )
	{
		TreePtr<AccessSpec> a;
		bool showtype=true;
		string sep2=";\n";
		shared_ptr<SCNamedRecord> scr = dynamic_pointer_cast< SCNamedRecord >(r);
		if( dynamic_pointer_cast< Class >(r) || scr )
		{
			s += "class";
			a = TreePtr<Private>(new Private);
		}
		else if( dynamic_pointer_cast< Struct >(r) )
		{
			s += "struct";
			a = TreePtr<Public>(new Public);
		}
		else if( dynamic_pointer_cast< Union >(r) )
		{
			s += "union";
			a = TreePtr<Public>(new Public);
		}
		else if( dynamic_pointer_cast< Enum >(r) )
		{
			s += "enum";
			a = TreePtr<Public>(new Public);
			sep2 = ",\n";
			showtype = false;
		}
		else
			return ERROR_UNSUPPORTED(declaration);

		// Name of the record
		s += " " + RenderIdentifier(r->identifier);

		if( !force_incomplete )
		{
			// Base classes
			if( TreePtr<InheritanceRecord> ir = dynamic_pointer_cast< InheritanceRecord >(declaration) )
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
					FOREACH( TreePtr<Base> b, ir->bases )
					{
						if( !first )
							s += ", ";
						first=false;
						ASSERT( b );
						s += RenderAccess(b->access) + " " /*+ RenderStorage(b->storage)*/ + RenderIdentifier(b->record);
					}
				}
			}

			// Contents
			AutoPush< TreePtr<Scope> > cs( scope_stack, r );
			s += "\n{\n" +
				 RenderDeclarationCollection( r, sep2, true, a, showtype ) +
				 "}";
		}

		s += ";\n";
	}
	else if( TreePtr<Label> l = dynamic_pointer_cast<Label>(declaration) )
		return RenderIdentifier(l->identifier) + ":;\n"; // need ; after a label in case last in compound block
	else
		s += ERROR_UNSUPPORTED(declaration);

		TRACE();
	return s;
}


string Render::RenderStatement( TreePtr<Statement> statement, string sep )
{
	TRACE();
	if( !statement )
		return sep;
	//printf( "%s %d things\n", typeid(*statement).name(), statement->Itemise().size() );
	if( TreePtr<Declaration> d = dynamic_pointer_cast< Declaration >(statement) )
		return RenderDeclaration( d, sep );
	else if( TreePtr<Compound> c = dynamic_pointer_cast< Compound >(statement) )
	{
		AutoPush< TreePtr<Scope> > cs( scope_stack, c );
		string s = "{\n";
		s += RenderDeclarationCollection( c, ";\n", true ); // Must do this first to populate backing list
		s += RenderSequence( c->statements, ";\n", true );
		return s + "}\n";
	}
	else if( TreePtr<Expression> e = dynamic_pointer_cast< Expression >(statement) )
		return RenderExpression(e) + sep;
	else if( TreePtr<Return> es = dynamic_pointer_cast<Return>(statement) )
		return "return " + RenderExpression(es->return_value) + sep;
	else if( TreePtr<Goto> g = dynamic_pointer_cast<Goto>(statement) )
	{
		if( TreePtr<SpecificLabelIdentifier> li = dynamic_pointer_cast< SpecificLabelIdentifier >(g->destination) )
			return "goto " + RenderIdentifier(li) + sep;  // regular goto
		else
			return "goto *(" + RenderExpression(g->destination) + ")" + sep; // goto-a-variable (GCC extension)
	}
	else if( TreePtr<If> i = dynamic_pointer_cast<If>(statement) )
	{
		string s;
		s += "if( " + RenderExpression(i->condition) + " )\n";
		bool sub_if = !!dynamic_pointer_cast<If>(i->body);
		if( sub_if )
			 s += "{\n"; // Note: braces there to clarify else binding eg if(a) if(b) foo; else how_do_i_bind;
	    s += RenderStatement(i->body, ";\n");
		if( sub_if )
			 s += "}\n";
		if( !dynamic_pointer_cast<Nop>(i->else_body) )  // Nop means no else clause
			s += "else\n" +
				 RenderStatement(i->else_body, ";\n");
		return s;
	}
	else if( TreePtr<While> w = dynamic_pointer_cast<While>(statement) )
		return "while( " + RenderExpression(w->condition) + " )\n" +
			   RenderStatement(w->body, ";\n");
	else if( TreePtr<Do> d = dynamic_pointer_cast<Do>(statement) )
		return "do\n" +
			   RenderStatement(d->body, ";\n") +
			   "while( " + RenderExpression(d->condition) + " )" + sep;
	else if( TreePtr<For> f = dynamic_pointer_cast<For>(statement) )
		return "for( " + RenderStatement(f->initialisation, "") + "; " + RenderExpression(f->condition) + "; "+ RenderStatement(f->increment, "") + " )\n" +
			   RenderStatement(f->body, ";\n");
	else if( TreePtr<Switch> s = dynamic_pointer_cast<Switch>(statement) )
		return "switch( " + RenderExpression(s->condition) + " )\n" +
			   RenderStatement(s->body, ";\n");
	else if( TreePtr<Case> c = dynamic_pointer_cast<Case>(statement) )
		return "case " + RenderExpression(c->value) + ":;\n";
	else if( TreePtr<RangeCase> rc = dynamic_pointer_cast<RangeCase>(statement) )
		return "case " + RenderExpression(rc->value_lo) + " ... " + RenderExpression(rc->value_hi) + ":\n";
	else if( dynamic_pointer_cast<Default>(statement) )
		return "default:;\n";
	else if( dynamic_pointer_cast<Continue>(statement) )
		return "continue" + sep;
	else if( dynamic_pointer_cast<Break>(statement) )
		return "break" + sep;
	else if( dynamic_pointer_cast<Nop>(statement) )
		return sep;
	else if( TreePtr<WaitDynamic> c = dynamic_pointer_cast<WaitDynamic>(statement) ) 
	    return c->GetToken() + "( " + RenderExpression(c->event) + " );\n";
	else if( TreePtr<WaitStatic> c = dynamic_pointer_cast<WaitStatic>(statement) ) 
	    return c->GetToken() + "();\n";
	else if( TreePtr<WaitDelta> c = dynamic_pointer_cast<WaitDelta>(statement) )
	    return c->GetToken() + "(SC_ZERO_TIME);\n";
	else if( TreePtr<NextTriggerDynamic> c = dynamic_pointer_cast<NextTriggerDynamic>(statement) ) 
	    return c->GetToken() + "( " + RenderExpression(c->event) + " );\n";
	else if( TreePtr<NextTriggerStatic> c = dynamic_pointer_cast<NextTriggerStatic>(statement) ) 
	    return c->GetToken() + "();\n";
	else if( TreePtr<NextTriggerDelta> c = dynamic_pointer_cast<NextTriggerDelta>(statement) ) 
	    return c->GetToken() + "(SC_ZERO_TIME);\n";
	else if( TreePtr<TerminationFunction> tf = dynamic_pointer_cast<TerminationFunction>(statement) )
		return tf->GetToken() + "( " + RenderExpression(tf->code) + " );\n";
	else if( TreePtr<NotifyImmediate> n = dynamic_pointer_cast<NotifyImmediate>(statement) )
		return RenderExpression( n->event, true ) + "." + n->GetToken() + "();\n";
	else if( TreePtr<NotifyDelta> n = dynamic_pointer_cast<NotifyDelta>(statement) )
		return RenderExpression( n->event, true ) + "." + n->GetToken() + "(SC_ZERO_TIME);\n";
    else
		return ERROR_UNSUPPORTED(statement);
}


template< class ELEMENT >
string Render::RenderSequence( Sequence<ELEMENT> spe,
					   string separator,
					   bool separate_last,
					   TreePtr<AccessSpec> init_access,
					   bool showtype )
{
	TRACE();
	string s;
	for( int i=0; i<spe.size(); i++ )
	{
		TRACE("%d %p\n", i, &i);
		string sep = (separate_last || i+1<spe.size()) ? separator : "";
		TreePtr<ELEMENT> pe = spe[i];
		if( TreePtr<Declaration> d = dynamic_pointer_cast< Declaration >(pe) )
			s += RenderDeclaration( d, sep, init_access ? &init_access : NULL, showtype );
		else if( TreePtr<Statement> st = dynamic_pointer_cast< Statement >(pe) )
			s += RenderStatement( st, sep );
		else
			s += ERROR_UNSUPPORTED(pe);
	}
	return s;
}


string Render::RenderOperandSequence( Sequence<Expression> spe,
							  string separator,
							  bool separate_last )
{
	TRACE();
	string s;
	for( int i=0; i<spe.size(); i++ )
	{
		TRACE("%d %p\n", i, &i);
		string sep = (separate_last || i+1<spe.size()) ? separator : "";
		TreePtr<Expression> pe = spe[i];
		s += RenderExpression( pe ) + sep;
	}
	return s;
}


string Render::RenderModuleCtor( TreePtr<Module> m,
                                 TreePtr<AccessSpec> *access )
{
    string s;
    
    // SystemC module, we must produce a constructor in SC style, do this as inline
    if( !dynamic_pointer_cast<Public>(*access) )
    {
        s += "public:\n";
        *access = MakeTreePtr<Public>();// note that we left the access as public
    }
    s += "SC_CTOR( " + RenderIdentifier( m->identifier ) + " )";
    int first = true;             
    FOREACH( TreePtr<Declaration> pd, m->members )
    {
        // Bodge an init list that names any fields we have that are modules
        // and initialises any fields with initialisers
        if( TreePtr<Field> f = dynamic_pointer_cast<Field>(pd) )
            if( TreePtr<TypeIdentifier> tid = dynamic_pointer_cast<TypeIdentifier>(f->type) )
                if( TreePtr<Record> r = GetRecordDeclaration(program, tid) )
                    if( dynamic_pointer_cast<Module>(r) )
                    {
                        if( first )
                            s += " :";
                        else
                            s += ",";
                        string ids = RenderIdentifier(f->identifier);                           
                        s += "\n" + ids + "(\"" + ids + "\")";
                        first = false;
                    }   
                    
        // TODO figure out what this does - it seems to look for function instances and then try to 
        // init them as if they wew initable and their body was an expression.
        if( TreePtr<Field> i = dynamic_pointer_cast<Field>(pd) )
        {
            TRACE("Got ")(*i)(" init is ")(*(i->initialiser))(" %d %d\n", 
                    (int)(bool)dynamic_pointer_cast<Callable>(i->type),
                    (int)(bool)dynamic_pointer_cast<Uninitialised>(i->initialiser) );                   
        
            if( !dynamic_pointer_cast<Callable>(i->type) && !dynamic_pointer_cast<Uninitialised>(i->initialiser) )
            {                   
                if( first )
                    s += " :";
                else
                    s += ",";
                string ids = RenderIdentifier(i->identifier);                           
                string inits = RenderExpression(i->initialiser);
                s += "\n" + ids + "(" + inits + ")";
                i->initialiser = MakeTreePtr<Uninitialised>(); // TODO naughty, changing the tree
                first = false;                 
            }
        }                      
    }    
    s += "\n{\n";
    FOREACH( TreePtr<Declaration> pd, m->members )
        if( TreePtr<Field> f = dynamic_pointer_cast<Field>(pd) )
            if( TreePtr<Process> r = dynamic_pointer_cast<Process>(f->type) )
                s += r->GetToken() + "(" + RenderIdentifier( f->identifier ) + ");\n";
    s += "}\n";
    
    return s;
}


string Render::RenderDeclarationCollection( TreePtr<Scope> sd,
									string separator,
									bool separate_last,
									TreePtr<AccessSpec> init_access,
									bool showtype )
{
	TRACE();

	// Uncomment one of these to stress the sorter
	//sd = ReverseDecls( sd );
	//sd = JumbleDecls( sd );

	Sequence<Declaration> sorted = SortDecls( sd->members, true );
	backing_ordering[sd] = sorted;

	// Emit an incomplete for each record
    string s;
	FOREACH( TreePtr<Declaration> pd, sorted ) //for( int i=0; i<sorted.size(); i++ )
		if( TreePtr<Record> r = dynamic_pointer_cast<Record>(pd) ) // is a record
			if( !dynamic_pointer_cast<Enum>(r) ) // but not an enum
				s += RenderDeclaration( r, separator, init_access ? &init_access : NULL, showtype, true );

    // For SystemC modules, we generate a constructor based on the other decls in
    // the module. Nothing goes in the Inferno tree for a module constructor, since
    // it is an elaboration mechanism, not funcitonal.
    if( TreePtr<Module> m = dynamic_pointer_cast<Module>(sd) )
        s += RenderModuleCtor( m, &init_access );

    // Emit the actual declarations, sorted for dependencies
    s += RenderSequence( sorted, separator, separate_last, init_access, showtype );
	TRACE();
	return s;
}

