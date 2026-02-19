
#include "cpptree.hpp"
#include "common/read_args.hpp"

#define EXPLICIT_BASE 0

using namespace CPPTree;

//////////////////////////// Uninitialised ///////////////////////////////

Syntax::Production Uninitialised::GetMyProductionTerminal() const
{
	// Most types don't use declarators, so provide a safe default
	return Production::ANONYMOUS; // Renders as an empty string
}

//////////////////////////// Type ///////////////////////////////

Syntax::Production Type::GetMyProductionTerminal() const
{
	// GetMyProductionTerminal on types is for non-booted anonymous types eg with auto a = new <here>;
	// Default shall be to boot (i.e. force parentheses).
	return Production::BOTTOM_EXPR;
}


Syntax::Production Type::GetOperandInDeclaratorProduction() const
{
	// Most types don't use declarators, so provide a safe default
	return Production::BOTTOM_EXPR;
}


string Type::GetRender( VN::RendererInterface *renderer, Production surround_prod, Policy policy )
{	
	// Declarator may be needed so enter declarator vcall but ask for anonymous by
	// setting declarator string to "". This corresponds to a type-id in https://alx71hub.github.io/hcb/ 
	return GetRenderTypeAndDeclarator( renderer, "", Syntax::Production::ANONYMOUS, surround_prod, policy, false ); 
}


string Type::GetRenderTypeAndDeclarator( VN::RendererInterface *renderer, string declarator, 
                                         Syntax::Production, Syntax::Production surround_prod, Syntax::Policy policy,
                                         bool constant)
{
	bool abstract = (declarator == "");
	Syntax::Production type_prod = abstract ? surround_prod  
                                               : Syntax::Production::SPACE_SEP_DECLARATION;
	return (constant?"const ":"") + 
	       GetRenderTypeSpecSeq( renderer, policy ) + 
	       (declarator != "" ? " "+declarator : "");
}                                           


string Type::GetRenderTypeSpecSeq( VN::RendererInterface *renderer, Policy policy )
{
	// This would be a type-specifier-seq in https://alx71hub.github.io/hcb/ 
	return renderer->RenderNodeExplicit(shared_from_this(), Syntax::Production::EXPLICIT_NODE, policy);
}

//////////////////////////// Declaration ///////////////////////////////

Syntax::Production Declaration::GetMyProductionTerminal() const
{
	return Production::BARE_DECLARATION;
}

//////////////////////////// Program ///////////////////////////////

Syntax::Production Program::GetMyProductionTerminal() const
{
	return Production::PROGRAM;
}

//////////////////////////// SpecificIdentifier ///////////////////////////////

SpecificIdentifier::SpecificIdentifier() :
    addr_bounding_role( BoundingRole::NONE )
{
}


SpecificIdentifier::SpecificIdentifier( string s, BoundingRole addr_bounding_role_ ) : 
    addr_bounding_role(addr_bounding_role_),
    name(s) 
{
}


shared_ptr<Cloner> SpecificIdentifier::Duplicate( shared_ptr<Cloner> p )
{
    return p; // duplicating specific identifiers just gets the same id, since they are unique.
    // This means x.Duplicate() matches x, wheras x.Clone() does not
}


bool SpecificIdentifier::IsLocalMatchCovariant( const Matcher &candidate ) const 
{
    return &candidate == this;
}


Orderable::Diff SpecificIdentifier::OrderCompare3WayCovariant( const Orderable &right, 
                                                               OrderProperty order_property ) const
{
	(void)order_property;
    auto &r = GET_THAT_REFERENCE(right);
        
    //FTRACEC("Compare ")(*this)(" with ")(*r)(": ");

    if( &r == this )
    {
        //FTRACEC("0 (fast out)\n");
        return 0; // fast-out
    }
        
    // Primary ordering on name due rule #528
    if( name != r.name )
    {
        //FTRACEC("%d (name)\n", name.compare(r->name));
        return name.compare(r.name);      
    }
          
    // Optional over-ride of address compare for making ranges, see rule #528
    if( addr_bounding_role != BoundingRole::NONE || r.addr_bounding_role != BoundingRole::NONE )
    {
        return (int)addr_bounding_role - (int)(r.addr_bounding_role);
    }    
    
    // Secondary ordering on identity due rule #528
    return Compare3WayIdentity(*this, r);
}


string SpecificIdentifier::GetRender( VN::RendererInterface *renderer, Production surround_prod, Policy policy) 
{		
	// Vcall on what kind of id this is
	string s = GetRenderWithoutScope(renderer, policy);          

    // Slight cheat for expediency: if a PURE_IDENTIFIER is expected, suppress scope resolution.
    // This could lead to the rendering of identifiers in the wrong scope. But, most PURE_IDENTIFIER
    // uses are declaring the id, or otherwise can't cope with the :: anyway. 
    if( surround_prod < Syntax::Production::PURE_IDENTIFIER ) 
    {
		auto me = TreePtr<SpecificIdentifier>::DynamicCast( TreePtr<Node>(shared_from_this()) );
		ASSERT(me);			
        s = renderer->RenderScopeResolvingPrefix( me ) + s;   
	}
                                     
    return s;
}


string SpecificIdentifier::GetIdentifierName() const
{
	return name;
}


string SpecificIdentifier::GetDesignationNameHint() const
{
	return GetIdentifierName();
}


bool SpecificIdentifier::IsDesignationNamedIdentifier() const
{
	return true;
}


string SpecificIdentifier::GetGraphName() const
{
    // Since this is text from the program, use single quotes
    string s = "'" + name + "'";
    switch( addr_bounding_role )
    {
        case BoundingRole::NONE:
            break;
        case BoundingRole::MINIMUS:
            s += " MINIMUS";
            break;
        case BoundingRole::MAXIMUS:
            s += " MAXIMUS";
            break;
    }    
    return s;
}


string SpecificIdentifier::GetTrace() const
{
    return GetName() + "(" + GetGraphName() + ")" + GetSerialString();
}


string SpecificIdentifier::GetRenderWithoutScope( VN::RendererInterface *renderer, Policy )
{
	auto me = TreePtr<SpecificIdentifier>::DynamicCast( TreePtr<Node>(shared_from_this()) );
	ASSERT(me);	

	string s = renderer->GetUniqueIdentifierName(me);          
    ASSERT(s.size()>0)(*me)(" rendered to an empty string\n");
    return s;
}

//////////////////////////// InstanceIdentifier //////////////////////////////

Syntax::Production InstanceIdentifier::GetMyProductionTerminal() const
{ 
	return Production::PURE_IDENTIFIER; 
}


//////////////////////////// SpecificConstructorIdentifier //////////////////////////////

// Constructors and destructors have identifiers because the tree is "resolved" which means
// we already know exactly which decl a function call should reach. Mediating the relationship via
// identifiers allows patterns to use couplings to get from usage to decl. These 
// identifiers render differently: where the name would go you have [Scope::][~]Class
// TODO we don't want the name we inherit from SpecificIdentifier so split a NamedIdentifier 
// out and use for named cases, but not these.

string SpecificConstructorIdentifier::GetRenderWithoutScope( VN::RendererInterface *renderer, Policy )
{
	auto me = TreePtr<SpecificConstructorIdentifier>::DynamicCast( TreePtr<Node>(shared_from_this()) );
	ASSERT(me);		
	
    // TODO use TryGetRecordDeclaration( Typeof( o->identifier ) ) and leave scopes out of it
    TreePtr<Record> rec = DynamicTreePtrCast<Record>( renderer->TryGetScope( me ) );
    if( !rec )
		throw Unimplemented(); // Constructor must be declared in a record       
		
    return renderer->DoRender( rec->identifier, Syntax::Production::PURE_IDENTIFIER );	// PURE_IDENTIFIER prevents scope resolution
}

//////////////////////////// SpecificDestructorIdentifier //////////////////////////////

string SpecificDestructorIdentifier::GetRenderWithoutScope( VN::RendererInterface *renderer, Policy )
{
	auto me = TreePtr<SpecificDestructorIdentifier>::DynamicCast( TreePtr<Node>(shared_from_this()) );
	ASSERT(me);		
	
    // TODO use TryGetRecordDeclaration( Typeof( o->identifier ) ) and leave scopes out of it
    TreePtr<Record> rec = DynamicTreePtrCast<Record>( renderer->TryGetScope( me ) );
    if( !rec )
		throw Unimplemented(); // Constructor must be declared in a record    
		   
    return "~" + 
           renderer->DoRender( rec->identifier, Syntax::Production::PURE_IDENTIFIER );	// PURE_IDENTIFIER prevents scope resolution
}

//////////////////////////// TypeIdentifier //////////////////////////////

Syntax::Production TypeIdentifier::GetMyProductionTerminal() const
{ 
	return Production::PURE_IDENTIFIER; 
}

//////////////////////////// SpecificTypeIdentifier //////////////////////////////

string SpecificTypeIdentifier::GetRender( VN::RendererInterface *renderer, Production surround_prod, Policy policy )
{
	// Render as identifier for cases that are not a type usage eg declarations
	return SpecificIdentifier::GetRender(renderer, surround_prod, policy);
}


string SpecificTypeIdentifier::GetRenderTypeSpecSeq( VN::RendererInterface *renderer, Policy policy )
{
	// Yes to scope resolution, otherwise we drop scope reolution on type usages
	return SpecificIdentifier::GetRender(renderer, Production::RESOLVER, policy);
}

//////////////////////////// IdValuePair ///////////////////////////////

Syntax::Production IdValuePair::GetMyProductionTerminal() const
{ 
	return Production::COLON_SEP; 
}


string IdValuePair::GetRender( VN::RendererInterface *renderer, Production, Policy )
{
    return renderer->DoRender( key, BoostPrecedence(Production::COLON_SEP) ) +
		   "⦂ " +
           renderer->DoRender( value, Production::COLON_SEP );	
}

//////////////////////////// MapArgumentation ///////////////////////////////

Syntax::Production MapArgumentation::GetMyProductionTerminal() const
{ 
	return Production::BRACKETED; 
}


string MapArgumentation::GetRender( VN::RendererInterface *, Production, Policy )
{
	throw Unimplemented(); // Not syntactically valid in isolation
}


string MapArgumentation::DirectRenderArgumentation(VN::RendererInterface *renderer, Policy policy)
{
	if( policy.refuse_map_argumentation )
		throw RefusedByPolicy(); // Would output 〔, 〕 and ⦂, so C++ renderer needs to resolve into seq args
			
	list<string> ls;
	for( TreePtr<Node> arg : arguments )
		ls.push_back( renderer->DoRender( arg, Production::COMMA_SEP ) );
	
    return Join( ls, ", ", "〔", "〕" );	
}

//////////////////////////// SeqArgumentation ///////////////////////////////

Syntax::Production SeqArgumentation::GetMyProductionTerminal() const
{ 
	return Production::BRACKETED; 
}


string SeqArgumentation::GetRender( VN::RendererInterface *, Production, Policy )
{	
	throw Unimplemented(); // Not syntactically valid in isolation
}


string SeqArgumentation::DirectRenderArgumentation(VN::RendererInterface *renderer, Policy)
{
	list<string> ls;
	for( TreePtr<Node> arg : arguments )
		ls.push_back( renderer->DoRender( arg, Production::COMMA_SEP ) );	
	
	return Join( ls, ", ", "(", ")" );	
}

//////////////////////////// Literal ///////////////////////////////

string Literal::GetName() const
{
	string value_string;
	try
	{
		value_string = "(" + GetRenderTerminal(Syntax::Production::BOTTOM_EXPR) + ")";
	}
	catch(Refusal &) {}
	return Traceable::GetName() + value_string;
}

//////////////////////////// SpecificString ///////////////////////////////

SpecificString::SpecificString() 
{
} 


SpecificString::SpecificString( string s ) :
      value(s) 
{
}


bool SpecificString::IsLocalMatchCovariant( const Matcher &candidate ) const 
{
    auto &c = GET_THAT_REFERENCE(candidate);
    return c.value == value;
}

 
Orderable::Diff SpecificString::OrderCompare3WayCovariant( const Orderable &right, 
                                                           OrderProperty ) const
{
    auto &r = GET_THAT_REFERENCE(right);
    return value.compare(r.value);
}
 
 
string SpecificString::GetRenderTerminal( Production ) const
{
    // Since this is a string literal, output it double quoted
    return "\"" + value + "\"";
}


Syntax::Production SpecificString::GetMyProductionTerminal() const
{ 
	return Production::PRIMARY_EXPR; 
}

//////////////////////////// SpecificInteger ///////////////////////////////

SpecificInteger::SpecificInteger()  : 
    value(TypeDb::int_bits, false)
{
}


SpecificInteger::SpecificInteger( llvm::APSInt i ) : 
    value(i)
{
}


SpecificInteger::SpecificInteger( int i ) : 
    value(TypeDb::int_bits, false)  // signed
{ 
    value = i; 
} 


SpecificInteger::SpecificInteger( int64_t i ) : 
    value(64, false)  // 64-bit
{ 
    value = i; 
} 


SpecificInteger::SpecificInteger( unsigned i ) : 
    value(TypeDb::int_bits, true)  // unsigned
{ 
    value = i; 
} 


SpecificInteger::SpecificInteger( uint64_t i ) : 
    value(64, false)  // unsigned
{ 
    value = i; 
} 


int64_t SpecificInteger::GetInt64() const
{
    return value.getSExtValue();
}


bool SpecificInteger::IsSigned() const
{
    return value.isSigned();
}


int64_t SpecificInteger::GetWidth() const
{
    return value.getBitWidth();
}


bool SpecificInteger::IsLocalMatchCovariant( const Matcher &candidate ) const
{
    auto &c = GET_THAT_REFERENCE(candidate);
    // A local match will require all fields to match, not just the numerical value.
    return c.value.isUnsigned() == value.isUnsigned() &&
           c.value.getBitWidth() == value.getBitWidth() &&
           c.value == value;
}


Orderable::Diff SpecificInteger::OrderCompare3WayCovariant( const Orderable &right, 
                                                            OrderProperty ) const
{
    auto &r = GET_THAT_REFERENCE(right);

    if( int d = ((int)value.isUnsigned() - (int)r.value.isUnsigned()) )
        return d;
    if( int d = (int)(value.getBitWidth()) - (int)(r.value.getBitWidth()) )
        return d;
    return (value > r.value) - (value < r.value);
    // Note: just subtracting could overflow
}
 
 
string SpecificInteger::GetRenderTerminal( Production ) const 
{
    return string(value.toString(10)) + // decimal
           (value.isUnsigned() ? "U" : "") +
#if EXPLICIT_BASE
           ("b"+to_string(value.getBitWidth()));
#else            
           (value.getBitWidth()>TypeDb::integral_bits[clang::DeclSpec::TSW_unspecified] ? "L" : "") +
           (value.getBitWidth()>TypeDb::integral_bits[clang::DeclSpec::TSW_long] ? "L" : "");
#endif
           // note, assuming longlong bigger than long, so second L appends first to get LL
}


Syntax::Production SpecificInteger::GetMyProductionTerminal() const
{ 
	return Production::PRIMARY_EXPR; 
}

//////////////////////////// SpecificFloat ///////////////////////////////

SpecificFloat::SpecificFloat() :
    llvm::APFloat(0.0) 
{
} 


SpecificFloat::SpecificFloat( llvm::APFloat v ) : 
    llvm::APFloat(v) 
{
}


bool SpecificFloat::IsLocalMatchCovariant( const Matcher &candidate ) const 
{
    auto &c = GET_THAT_REFERENCE(candidate);
    // A local match will require all fields to match, not just the numerical value.
    return bitwiseIsEqual( c ); 
    //     && c.getSemantics() == getSemantics() && //TODO
    
}


Orderable::Diff SpecificFloat::OrderCompare3WayCovariant( const Orderable &right, 
                                                          OrderProperty order_property ) const
{
	(void)order_property;
    auto &r = GET_THAT_REFERENCE(right);
        
    // Primary ordering: the value
    cmpResult cr = compare(r);
    if( cr==APFloat::cmpLessThan )
        return -1;
    if( cr==APFloat::cmpGreaterThan )
        return 1;    
    
    // Secondary ordering: the hash
    uint32_t h = getHashValue();
    uint32_t ch = r.getHashValue();
    return (int)(h > ch) - (int)(h < ch);
    // Note: just subtracting could overflow
}
 

string SpecificFloat::GetRenderTerminal( Production ) const
{
    char hs[256];
    // generate hex float since it can be exact
    convertToHexString( hs, 0, false, llvm::APFloat::rmTowardNegative); // note rounding mode ignored when hex_digits==0
    return string(hs) +
           (&getSemantics()==TypeDb::float_semantics ? "F" : "") +
           (&getSemantics()==TypeDb::long_double_semantics ? "L" : "");
}


Syntax::Production SpecificFloat::GetMyProductionTerminal() const
{ 
	return Production::PRIMARY_EXPR; 
}

//////////////////////////// AccessSpec //////////////////////////////

Syntax::Production AccessSpec::GetMyProductionTerminal() const
{ 
	return Production::TERMINAL; 
}

//////////////////////////// Instance //////////////////////////////

Syntax::Production Instance::GetMyProduction(const VN::RendererInterface *, Policy policy) const
{ 
	if( !DynamicTreePtrCast<Expression>(initialiser) && policy.force_initialisation )
		return Production::DECLARATION;
	else
		return Production::BARE_DECLARATION;
}

//////////////////////////// LabelIdentifier //////////////////////////////

Syntax::Production LabelIdentifier::GetMyProductionTerminal() const
{ 
	return Production::PURE_IDENTIFIER; 
}

//////////////////////////// SpecificLabelIdentifier //////////////////////////////

Syntax::Production SpecificLabelIdentifier::GetMyProductionTerminal() const
{ 
	return Production::PURE_IDENTIFIER; 
}


string SpecificLabelIdentifier::GetRender( VN::RendererInterface *renderer, Production surround_prod, Policy policy)
{
	if( policy.goto_uses_ref_and_deref && 
	    surround_prod < Syntax::Production::PURE_IDENTIFIER ) // Don't add && in a label declaration
	{
		// label-as-variable (GCC extension)  
		return "&&" + SpecificIdentifier::GetRender( renderer, Syntax::Production::RESOLVER, policy );
	}		   
    
    return SpecificIdentifier::GetRender( renderer, surround_prod, policy );
}

//////////////////////////// LabelDeclaration //////////////////////////////

Syntax::Production LabelDeclaration::GetMyProductionTerminal() const
{ 
	return Production::LABEL; 
}


string LabelDeclaration::GetRender( VN::RendererInterface *renderer, Production, Policy )
{
	return renderer->DoRender( identifier, Syntax::Production::PURE_IDENTIFIER) + ":";	
}

//////////////////////////// Callable //////////////////////////////

Syntax::Production Callable::GetOperandInDeclaratorProduction() const
{
	return Production::POSTFIX; // eg int a();
}


//////////////////////////// CallableParams //////////////////////////////

string CallableParams::GetRenderParams(VN::RendererInterface *renderer, Policy policy)
{
	TRACE();
    Sequence<Declaration> sorted;
    for( auto param : params )
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
            s += renderer->RenderNodeExplicit( d, Syntax::Production::BARE_DECLARATION, policy );
            continue;
        }
        Syntax::Production starting_declarator_prod = Syntax::Production::PURE_IDENTIFIER;
        string name = renderer->DoRender( o->identifier, starting_declarator_prod);
        s += renderer->DoRenderTypeAndDeclarator( o->type, name, starting_declarator_prod, Syntax::Production::BARE_DECLARATION, policy, false );
            
        first = false;
    }
    return s;
}

//////////////////////////// Function //////////////////////////////

string Function::GetRenderTypeAndDeclarator( VN::RendererInterface *renderer, string declarator, 
											 Syntax::Production , Syntax::Production surround_prod, Syntax::Policy policy,
											 bool constant )
{
	string d2 = declarator + 
	            "(" + 
	            GetRenderParams(renderer, policy) +
	            ")" + 
	            (constant?" const":"");
    return renderer->DoRenderTypeAndDeclarator( return_type, 
                                                d2,
                                                Syntax::Production::POSTFIX, 
                                                surround_prod, 
                                                policy );
}

//////////////////////////// Constructor //////////////////////////////

string Constructor::GetRender( VN::RendererInterface *renderer, Production, Policy policy)
{
	// TODO obviously the class-being-constructed needs to be added to this syntax, either by
	// 1 add it to this node, which is a type, so that the type of a constructor includes the type it constructs
	// 2 pass it in, which requires a new kind of virtual rendering method
	// 3 vcall back to renderer to get the scope, make sure works in line and out of line
	// TODO don't render ⨤ in C++. Maybe not at all if can parse VN without.
	return "⨤(" + GetRenderParams(renderer, policy) + ")";
}


string Constructor::GetRenderTypeAndDeclarator( VN::RendererInterface *renderer, string declarator, 
												Syntax::Production , Syntax::Production, Syntax::Policy policy,
												bool )
{
	ASSERT(!declarator.empty()); // must NOT be anonymous
	return declarator + "(" + GetRenderParams(renderer, policy) + ")";
}

//////////////////////////// Destructor //////////////////////////////

string Destructor::GetRender( VN::RendererInterface *, Production, Policy )
{
	// TODO see Constructor::GetRender()
	return "~()";
}


string Destructor::GetRenderTypeAndDeclarator( VN::RendererInterface *, string declarator, 
											   Syntax::Production , Syntax::Production, Syntax::Policy,
											   bool )
{
	ASSERT(!declarator.empty()); // must NOT be anonymous
	return declarator + "()";
}

//////////////////////////// Array //////////////////////////////

Syntax::Production Array::GetMyProductionTerminal() const
{ 
	return Production::POSTFIX; // eg auto a = new int[9]; (without booting)
}


Syntax::Production Array::GetOperandInDeclaratorProduction() const
{
	return Production::POSTFIX; // eg int a[9];
}


string Array::GetRenderTypeAndDeclarator( VN::RendererInterface *renderer, string declarator, 
                                   Syntax::Production, Syntax::Production surround_prod, Syntax::Policy policy,
                                   bool constant )
{
	string d2 = declarator + 
	            "[" + 
	            renderer->DoRender( size, Syntax::Production::BOTTOM_EXPR) + 
	            "]";
    return renderer->DoRenderTypeAndDeclarator( element, 
												d2, 
												Syntax::Production::POSTFIX,
												surround_prod,
												policy,
												constant );
}                                   

//////////////////////////// Indirection //////////////////////////////

Syntax::Production Indirection::GetMyProductionTerminal() const
{ 
	return Production::POSTFIX; // eg auto a = new int *; (without booting)
}


Syntax::Production Indirection::GetOperandInDeclaratorProduction() const
{
	return Production::PREFIX; // eg int *a;
}

//////////////////////////// Pointer //////////////////////////////

string Pointer::GetRenderTypeAndDeclarator( VN::RendererInterface *renderer, string declarator, 
											Syntax::Production, Syntax::Production surround_prod, Syntax::Policy policy,
											bool constant )
{
	// TODO Pointer node to indicate constancy of pointed-to object - would go into this call to DoRenderTypeAndDeclarator
	string d2 = string(DynamicTreePtrCast<Const>(constancy)?"const ":"") + "*" + (constant?" const ":"") + declarator;
    return renderer->DoRenderTypeAndDeclarator( destination, 
											    d2,
											    Syntax::Production::PREFIX, 
											    surround_prod, 
											    policy, 
											    false ); 
}                                       

//////////////////////////// Reference //////////////////////////////

string Reference::GetRenderTypeAndDeclarator( VN::RendererInterface *renderer, string declarator, 
											Syntax::Production, Syntax::Production surround_prod, Syntax::Policy policy,
											bool constant )
{
	string d2 = string(DynamicTreePtrCast<Const>(constancy)?"const ":"") + "&" + (constant?" const ":"") + declarator;
    return renderer->DoRenderTypeAndDeclarator( destination, 
											    d2,
											    Syntax::Production::PREFIX, 
											    surround_prod, 
											    policy, 
											    false ); 
}                                       

//////////////////////////// Void ///////////////////////////////

Syntax::Production Void::GetMyProductionTerminal() const
{ 
	return Production::PRIMARY_TYPE; // eg auto a = new void;
}


string Void::GetRenderTypeSpecSeq( VN::RendererInterface *, Policy )
{
	return "void";
}

//////////////////////////// Boolean ///////////////////////////////

Syntax::Production Boolean::GetMyProductionTerminal() const
{ 
	return Production::PRIMARY_TYPE; // eg auto a = new bool;
}

string Boolean::GetRenderTypeSpecSeq( VN::RendererInterface *, Policy )
{
	return "bool";
}

//////////////////////////// Numeric ///////////////////////////////

Syntax::Production Numeric::GetMyProductionTerminal() const
{ 
	return Production::SPACE_SEP_TYPE; // eg auto a = new unsigned long;
}

//////////////////////////// Integral ///////////////////////////////

string Integral::GetRenderTypeAndDeclarator( VN::RendererInterface *renderer, string declarator, 
                                             Syntax::Production, Syntax::Production, Syntax::Policy policy,
                                             bool constant)
{
    int64_t width_val;
    auto ic = DynamicTreePtrCast<SpecificInteger>( width );
    if( !ic )
		throw Unimplemented(); // Cannot render because width was wildcarded (using NULL or Integer node)
		
    width_val = ic->GetInt64();

    TRACE("width %" PRId64 "\n", width_val);

	string s = GetRenderTypeSpecSeq( renderer, policy ); // TODO could we not recurse via DoRenderTypeAndDeclarator() - once we've made the declarator string?

    s += " " + declarator;

    // Fix the width
    bool bitfield = !( width_val == TypeDb::char_bits ||
                       width_val == TypeDb::integral_bits[clang::DeclSpec::TSW_short] ||
                       width_val == TypeDb::integral_bits[clang::DeclSpec::TSW_unspecified] ||
                       width_val == TypeDb::integral_bits[clang::DeclSpec::TSW_long] ||
                       width_val == TypeDb::integral_bits[clang::DeclSpec::TSW_longlong] );

    if( bitfield )
    {
		// This function only exists to provide bitfields in member declarations that use declarators.
		// RenderSimpleTypeIntegral() can provide the pure types directly, without bitfields.
		s += SSPrintf(":%" PRId64, width_val);
    }

	return (constant?"const ":"") + s;
}


string Integral::GetRenderTypeSpecSeq( VN::RendererInterface *, Policy )
{
    bool ds;
    int64_t width_bits;
    auto ic = DynamicTreePtrCast<SpecificInteger>( width );
    if(!ic)
		throw UnimplemetedIntegralType();
    width_bits = ic->GetInt64();

    TRACE("width %" PRId64 "\n", width);

    if( width_bits == TypeDb::char_bits )
        ds = TypeDb::char_default_signed;
    else
        ds = true;

    // Produce signed or unsigned if required
    // Note: literal strings can be converted to char * but not unsigned char * or signed char *
    bool is_signed = IsSigned();
    string s;
    if( is_signed && !ds )
        s = "signed ";
    else if( !is_signed && ds )
        s = "unsigned ";

    // Fix the width
    if( width_bits <= TypeDb::char_bits )
        s += "char";
    else if( width_bits <= TypeDb::integral_bits[clang::DeclSpec::TSW_short] )
        s += "short";
    else if( width_bits <= TypeDb::integral_bits[clang::DeclSpec::TSW_unspecified] )
        s += "int";
    else if( width_bits <= TypeDb::integral_bits[clang::DeclSpec::TSW_long] )
        s += "long";
    else if( width_bits <= TypeDb::integral_bits[clang::DeclSpec::TSW_longlong] )
        s += "long long";
    else
		throw UnimplemetedIntegralType();
		
	if( ReadArgs::use.count("c") )
		s += SSPrintf("/* %d bits */", width_bits );

    return s;	
}

//////////////////////////// SpecificFloatSemantics ///////////////////////////////

SpecificFloatSemantics::SpecificFloatSemantics() 
{
}


SpecificFloatSemantics::SpecificFloatSemantics( const llvm::fltSemantics *s ) : 
    value(s)
{
}


bool SpecificFloatSemantics::IsLocalMatchCovariant( const Matcher &candidate ) const
{
    auto &c = GET_THAT_REFERENCE(candidate);
    return c.value == value;
}


Orderable::Diff SpecificFloatSemantics::OrderCompare3WayCovariant( const Orderable &right, 
                                                                   OrderProperty order_property ) const
{
    auto &r = GET_THAT_REFERENCE(right); 

    Orderable::Diff d;
    switch( order_property )
    {
    case TOTAL:
        // Don't use any particular ordering apart from where the 
        // llvm::fltSemantics are being stored.
        d = (int)(value > r.value) - (int)(value < r.value);
        // Note: just subtracting could overflow
        break;
    case REPEATABLE:
    default:
        // Repeatable ordering stops at type
        d = 0;
        break;
    }
    return d;
}


SpecificFloatSemantics::operator const llvm::fltSemantics &() const 
{
    return *value;
}

//////////////////////////// Floating ///////////////////////////////

string Floating::GetRenderTypeSpecSeq( VN::RendererInterface *, Policy )
{
    string s;
    TreePtr<SpecificFloatSemantics> sem = DynamicTreePtrCast<SpecificFloatSemantics>(semantics);
    ASSERT(sem);

    if( &(const llvm::fltSemantics &)*sem == TypeDb::float_semantics )
        s += "float";
    else if( &(const llvm::fltSemantics &)*sem == TypeDb::double_semantics )
        s += "double";
    else if( &(const llvm::fltSemantics &)*sem == TypeDb::long_double_semantics )
        s += "long double";
    else
        throw UnimplemetedFloatingType();

    return s;
}

//////////////////////////// Labeley ///////////////////////////////

Syntax::Production Labeley::GetMyProductionTerminal() const
{
	return Production::POSTFIX; // renders as void *
}

string Labeley::GetRenderTypeSpecSeq( VN::RendererInterface *, Policy policy )
{
	if( policy.refuse_local_node_types ) 
		throw RefuseDueLocal(); 
	return "const void *"; // TODO lower Labely to const void * in a lowering pass
}

//////////////////////////// Typedef ///////////////////////////////

Syntax::Production Typedef::GetMyProductionTerminal() const
{
	return Production::BARE_DECLARATION; 
}

//////////////////////////// Record ///////////////////////////////

Syntax::Production Record::GetMyProductionTerminal() const
{
	return Production::BARE_DECLARATION;
}

TreePtr<AccessSpec> Record::GetInitialAccess() const
{
	return nullptr;
}

//////////////////////////// Union ///////////////////////////////

TreePtr<AccessSpec> Union::GetInitialAccess() const
{
	return MakeTreeNode<Public>();
}

//////////////////////////// Struct ///////////////////////////////

TreePtr<AccessSpec> Struct::GetInitialAccess() const
{
	return MakeTreeNode<Public>();
}

//////////////////////////// Class ///////////////////////////////

TreePtr<AccessSpec> Class::GetInitialAccess() const
{
	return MakeTreeNode<Private>();
}

//////////////////////////// True ///////////////////////////////

Syntax::Production True::GetMyProductionTerminal() const
{ 
	return Production::PRIMARY_EXPR; 
}

//////////////////////////// False ///////////////////////////////

Syntax::Production False::GetMyProductionTerminal() const
{ 
	return Production::PRIMARY_EXPR; 
}

//////////////////////////// Operators from operator_data.inc ///////////////////////////////

#define PREFIX(TOK, TEXT, NODE, BASE, CAT, PROD, ASSOC) \
Syntax::Production NODE::GetMyProductionTerminal() const \
{ \
	return Production::PROD; \
} \
string NODE::GetRender( VN::RendererInterface *renderer, Production, Policy ) \
{ \
	Sequence<Expression>::iterator operands_it = operands.begin(); \
	string s = TEXT; \
	bool paren = false; \
	/* Prevent interpretation as a member function pointer literal */ \
	if( dynamic_cast<AddressOf *>(shared_from_this().get()) ) \
		if( auto id = TreePtr<Identifier>::DynamicCast(*operands_it) ) \
			paren = !renderer->RenderScopeResolvingPrefix( id ).empty(); \
	return s + (paren?"(":"") + renderer->DoRender( *operands_it, Production::PROD) + (paren?")":""); \
} \

#define POSTFIX(TOK, TEXT, NODE, BASE, CAT, PROD, ASSOC) \
Syntax::Production NODE::GetMyProductionTerminal() const \
{ \
	return Production::PROD; \
} \
string NODE::GetRender( VN::RendererInterface *renderer, Production, Policy ) \
{ \
	Sequence<Expression>::iterator operands_it = operands.begin(); \
	return renderer->DoRender( *operands_it, Production::PROD) + TEXT; \
} \

#define INFIX(TOK, TEXT, NODE, BASE, CAT, PROD, ASSOC) \
Syntax::Production NODE::GetMyProductionTerminal() const \
{ \
	return Production::PROD; \
} \
string NODE::GetRender( VN::RendererInterface *renderer, Production, Policy ) \
{ \
	Production prod_left = Production::PROD; \
	Production prod_right = Production::PROD; \
	switch( Association::ASSOC ) \
	{ \
		case Association::RIGHT: prod_left = BoostPrecedence(prod_left); break; \
		case Association::LEFT:  prod_right = BoostPrecedence(prod_right); break; \
	} \
	Sequence<Expression>::iterator operands_it = operands.begin(); \
	string s = renderer->DoRender( *operands_it, prod_left ); \
	s += TEXT; \
	++operands_it; \
	s += renderer->DoRender( *operands_it, prod_right ); \
	return s; \
}

#include "operator_data.inc"

//////////////////////////// ConditionalOperator ///////////////////////////////

Syntax::Production ConditionalOperator::GetMyProductionTerminal() const
{ 
	// This operator borros ASSIGN's precidence and both are right-associative, 
	// so the two can be mingled freely and will always evaluate right-to-left.
	return Production::ASSIGN; 
}


string ConditionalOperator::GetRender( VN::RendererInterface *renderer, Production, Policy )
{
	return renderer->DoRender( condition, BoostPrecedence(Syntax::Production::ASSIGN) ) + 
		   " ? " +
		   // Middle expression boots parser - so you can't split it up using (), [] etc
		   renderer->DoRender( expr_then, Production::BOTTOM_EXPR ) + 
		   " : " +
		   renderer->DoRender( expr_else, Production::ASSIGN );          
}


//////////////////////////// Subscript ///////////////////////////////

Syntax::Production Subscript::GetMyProductionTerminal() const
{ 
	return Production::POSTFIX; 
}

string Subscript::GetRender( VN::RendererInterface *renderer, Production, Policy )
{
	return renderer->DoRender( destination, Production::POSTFIX ) + 
		   "[" +
		   renderer->DoRender( index, Production::BOTTOM_EXPR ) + 
		   "]";       
}

//////////////////////////// ArrayLiteral ///////////////////////////////

Syntax::Production ArrayLiteral::GetMyProductionTerminal() const
{ 
	return Production::INITIALISER; 
}


string ArrayLiteral::GetRender( VN::RendererInterface *renderer, Production, Policy )
{
	list<string> renders;    
    for( TreePtr<Expression> e : elements )
		renders.push_back( renderer->DoRender( e, Production::COMMA_SEP ) );
    // Use of ={} in expressions is irregular so handle locally. = is used to disambiguate
    // from a compound statement.
    return "=" + Join(renders, ", ", "{", "}"); 
}

//////////////////////////// This ///////////////////////////////

Syntax::Production This::GetMyProductionTerminal() const
{ 
	return Production::PRIMARY_EXPR; 
}


string This::GetRenderTerminal( Production ) const
{
	return "this";
}

//////////////////////////// New ///////////////////////////////

Syntax::Production New::GetMyProductionTerminal() const
{ 
	return Production::PREFIX; 
}


string New::GetRender( VN::RendererInterface *renderer, Production, Policy policy)
{
	return string (DynamicTreePtrCast<Global>(global) ? "::" : "") +
		   "new" + placement_argumentation->DirectRenderArgumentation(renderer, policy) +
		   " " +
		   renderer->DoRender( type, Syntax::Production::TYPE_IN_NEW ) +
		   constructor_argumentation->DirectRenderArgumentation(renderer, policy);
}

//////////////////////////// Delete ///////////////////////////////

Syntax::Production Delete::GetMyProductionTerminal() const
{ 
	return Production::PREFIX; 
}


string Delete::GetRender( VN::RendererInterface *renderer, Production, Policy )
{
	return string(DynamicTreePtrCast<Global>(global) ? "::" : "") +
		   "delete" +
		   (DynamicTreePtrCast<DeleteArray>(array) ? "[]" : "") +
		   " " + renderer->DoRender( pointer, Syntax::Production::PREFIX );
}


//////////////////////////// Lookup ///////////////////////////////

Syntax::Production Lookup::GetMyProductionTerminal() const
{ 
	return Production::POSTFIX; 
}


string Lookup::GetRender( VN::RendererInterface *renderer, Production , Policy  )
{
	return renderer->DoRender(object, Production::POSTFIX) +
		   "." +
		   renderer->DoRender(member, Production::PRIMARY_EXPR);
}

//////////////////////////// Cast ///////////////////////////////

Syntax::Production Cast::GetMyProductionTerminal() const
{ 
	return Production::PREFIX; 
}

string Cast::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	if( policy.refuse_c_style_cast )
		throw RefusedByPolicy();
    return "(" + renderer->DoRender( type, Syntax::Production::BOOT_TYPE ) + ")" +
                 renderer->DoRender( operand, Syntax::Production::PREFIX );
}

//////////////////////////// Call ///////////////////////////////

Syntax::Production Call::GetMyProductionTerminal() const
{
	return Production::POSTFIX; 	
}


string Call::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{				
	return renderer->DoRender( callee, Syntax::Production::POSTFIX ) + 
	       argumentation->DirectRenderArgumentation(renderer, policy);
}

//////////////////////////// ConstructInit ///////////////////////////////

Syntax::Production ConstructInit::GetMyProductionTerminal() const
{
	return Production::INITIALISER; 	
}


string ConstructInit::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{		
	if( !policy.detect_and_render_constructor )
		throw RefusedByPolicy(); // TODO find a way of disambiguating from a Call in VN lang
			
	// We never render the identifier for constructors - they are "invisible" and represent
	// the choice of which overload we are bound to.		
	return argumentation->DirectRenderArgumentation(renderer, policy);
}

//////////////////////////// RecordLiteral ///////////////////////////////

Syntax::Production RecordLiteral::GetMyProductionTerminal() const
{ 
	return Production::BRACKETED; 
}

//////////////////////////// SizeOf ///////////////////////////////

Syntax::Production SizeOf::GetMyProductionTerminal() const
{ 
	return Production::PREFIX; 	
}


//////////////////////////// AlignOf ///////////////////////////////

Syntax::Production AlignOf::GetMyProductionTerminal() const
{ 
	return Production::PREFIX; 
}

//////////////////////////// Compound ///////////////////////////////

Syntax::Production Compound::GetMyProductionTerminal() const
{ 
	return Production::COMPOUND;
}


string Compound::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
    string s = " { ";
    for( TreePtr<Declaration> m : members )    
        s += renderer->DoRender( m, Syntax::Production::DECLARATION, policy );    
    if( policy.compound_uses_vn_separator )
		s += "⚬";
    for( TreePtr<Statement> st : statements )    
        s += renderer->DoRender( st, Syntax::Production::STATEMENT_LOW, policy );    
    s += " } ";
    return s;
}

//////////////////////////// StatementExpression ///////////////////////////////

Syntax::Production StatementExpression::GetMyProductionTerminal() const
{ 
	return Production::BRACKETED; 
}

/*
string StatementExpression::GetRender( VN::RendererInterface *renderer, Production production, Policy policy )
{
}*/

//////////////////////////// Return ///////////////////////////////

Syntax::Production Return::GetMyProductionTerminal() const
{ 
	return Production::BARE_STATEMENT; 	
}


string Return::GetRender( VN::RendererInterface *renderer, Production, Policy )
{
	return "return " + renderer->DoRender( return_value, Syntax::Production::SPACE_SEP_STATEMENT );
}

//////////////////////////// Goto ///////////////////////////////

Syntax::Production Goto::GetMyProductionTerminal() const
{ 
	return Production::BARE_STATEMENT; 
}


string Goto::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	string s = "goto ";
	bool star = false;
	bool remove_double_deref = false;
	Production prod = Production::SPACE_SEP_STATEMENT;
    if( policy.goto_uses_ref_and_deref )
    {
		if( !DynamicTreePtrCast< SpecificLabelIdentifier >(destination) )
			star = true;
		else
			remove_double_deref = true;
	}
	
	if( star )
	{
		s += "*";
		prod = Production::PREFIX;
	}
	
	string label = renderer->DoRender( destination, prod );
	
	if( remove_double_deref )
		label = label.substr(2); // REMOVE THE &&
	
	return s + label;
}

//////////////////////////// If ///////////////////////////////

Syntax::Production If::GetMyProductionTerminal() const
{ 
	// If we don't have an else clause, we might steal the else from a 
	// surrounding If node, so drop our precedence a little bit.
	bool has_else_clause = !DynamicTreePtrCast<Nop>(body_else);
	return has_else_clause ? Production::STATEMENT_HIGH : Production::STATEMENT_LOW; 
}


string If::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	bool has_else_clause = !DynamicTreePtrCast<Nop>(body_else); // Nop means no else clause
	string s = "if( " +
		       renderer->DoRender( condition, Production::BOTTOM_EXPR, policy ) +
		       " )\n" +
		       renderer->DoRender( body, has_else_clause ? Production::STATEMENT_HIGH : Production::STATEMENT_LOW, policy );
		       
    if( has_else_clause )  
        s += "else\n" + renderer->DoRender( body_else, Production::STATEMENT_LOW);
        
    return s;		       
}

//////////////////////////// Breakable ///////////////////////////////

Syntax::Production Breakable::GetMyProductionTerminal() const
{ 
	return Production::STATEMENT_HIGH; 
}

//////////////////////////// While ///////////////////////////////

Syntax::Production While::GetMyProductionTerminal() const
{ 
	return Production::STATEMENT; 
}


string While::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return "while( " +
		   renderer->DoRender( condition, Production::BOTTOM_EXPR, policy ) +
		   " )\n" +
		   renderer->DoRender( body, Production::STATEMENT, policy );
}

//////////////////////////// Do ///////////////////////////////

Syntax::Production Do::GetMyProductionTerminal() const
{ 
	return Production::BARE_STATEMENT; 
}


string Do::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return "do\n" +
		   renderer->DoRender( body, Production::STATEMENT_LOW, policy ) +
		   "while( " +
		   renderer->DoRender( condition, Production::BOTTOM_EXPR, policy ) +
		   " )" ;
}

//////////////////////////// For ///////////////////////////////

Syntax::Production For::GetMyProductionTerminal() const
{ 
	return Production::STATEMENT; 
}


string For::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return "for( " +
		   renderer->DoRender( initialisation, Production::BOTTOM_EXPR, policy ) +
		   "; " +
		   renderer->DoRender( condition, Production::BOTTOM_EXPR, policy ) +
		   "; " +
		   renderer->DoRender( increment, Production::BOTTOM_EXPR, policy ) +
		   " )\n" +
		   renderer->DoRender( body, Production::STATEMENT, policy );
}

//////////////////////////// Switch ///////////////////////////////

Syntax::Production Switch::GetMyProductionTerminal() const
{ 
	return Production::STATEMENT; 
}


string Switch::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return "switch( " +
		   renderer->DoRender( condition, Production::BOTTOM_EXPR, policy ) +
		   " )\n" +
		   renderer->DoRender( body, Production::STATEMENT, policy );
}

//////////////////////////// SwitchTarget ///////////////////////////////

Syntax::Production SwitchTarget::GetMyProductionTerminal() const
{ 
	return Production::LABEL; 
}

//////////////////////////// RangeCase //////////////////////////////

string RangeCase::GetRender( VN::RendererInterface *renderer, Production, Policy )
{
	return "case " + 
	       renderer->DoRender( value_lo, Syntax::Production::EXPR_CONST) + 
	       ".." +
	       renderer->DoRender( value_hi, Syntax::Production::EXPR_CONST) + 
	       ":";	
}

//////////////////////////// Case //////////////////////////////

string Case::GetRender( VN::RendererInterface *renderer, Production, Policy )
{
	return "case " + renderer->DoRender( value, Syntax::Production::EXPR_CONST) + ":";	
}

//////////////////////////// Default //////////////////////////////

string Default::GetRender( VN::RendererInterface *, Production, Policy )
{
	return "default:";	
}

//////////////////////////// Continue ///////////////////////////////

Syntax::Production Continue::GetMyProductionTerminal() const
{ 
	return Production::BARE_STATEMENT; 
}


string Continue::GetRenderTerminal( Production ) const
{
	return "continue";
}


//////////////////////////// Break ///////////////////////////////

Syntax::Production Break::GetMyProductionTerminal() const
{ 
	return Production::BARE_STATEMENT; 
}


string Break::GetRenderTerminal( Production ) const
{
	return "break";
}

//////////////////////////// MembInitialisation ///////////////////////////////

Syntax::Production MembInitialisation::GetMyProductionTerminal() const
{ 
	return Production::COMMA_SEP; 
}


string MembInitialisation::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	if( !policy.detect_and_render_constructor )
		throw RefusedByPolicy(); // TODO find a way of disambiguating from a Call in VN lang

	return renderer->DoRender( member_id, Production::PURE_IDENTIFIER, policy ) +
		   renderer->DoRender( initialiser, Production::INITIALISER, policy );
}

//////////////////////////// Nop ///////////////////////////////

Syntax::Production Nop::GetMyProductionTerminal() const
{ 
	return Production::BARE_STATEMENT; // Force a ;
}


string Nop::GetRender( VN::RendererInterface *, Production, Policy )
{
	return "";
}

//////////////////////////// PreprocessorIdentifier //////////////////////////////

Syntax::Production PreprocessorIdentifier::GetMyProductionTerminal() const
{ 
	return Production::PURE_IDENTIFIER; 
}

//////////////////////////// MacroDeclaration ///////////////////////////////

Syntax::Production MacroDeclaration::GetMyProduction(const VN::RendererInterface *, Policy) const
{ 
	if( !DynamicTreePtrCast<Expression>(initialiser) )
		return Production::DECLARATION;
	else
		return Production::BARE_DECLARATION;
}

//////////////////////////// MacroStatement ///////////////////////////////

Syntax::Production MacroStatement::GetMyProductionTerminal() const
{ 
	return Production::STATEMENT; 
}

//////////////////////////// PreProcDecl ///////////////////////////////

Syntax::Production PreProcDecl::GetMyProductionTerminal() const
{ 
	return Production::PRE_PROC_DIRECTIVE; 
}

