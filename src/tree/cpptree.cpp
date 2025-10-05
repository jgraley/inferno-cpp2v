
#include "cpptree.hpp"

#define EXPLICIT_BASE 0

using namespace CPPTree;

//////////////////////////// Uninitialised ///////////////////////////////

Syntax::Production Uninitialised::GetMyProduction() const
{
	// Most types don't use declarators, so provide a safe default
	return Production::ANONYMOUS; // Renders as an empty string
}

//////////////////////////// Type ///////////////////////////////

Syntax::Production Type::GetMyProduction() const
{
	// GetMyProduction on types is for non-booted anonymous types eg with auto a = new <here>;
	// Default shall be to boot (i.e. force parentheses).
	return Production::BOOT_EXPR;
}


Syntax::Production Type::GetOperandInDeclaratorProduction() const
{
	// Most types don't use declarators, so provide a safe default
	return Production::BOOT_EXPR;
}

//////////////////////////// Declaration ///////////////////////////////

Syntax::Production Declaration::GetMyProduction() const
{
	return Production::DECLARATION;
}

//////////////////////////// Program ///////////////////////////////

Syntax::Production Program::GetMyProduction() const
{
	return Production::PROGRAM;
}

//////////////////////////// Literal ///////////////////////////////

string Literal::GetName() const
{
    return Traceable::GetName() + "(" + GetToken() + ")";
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
 
 
string SpecificString::GetToken() const
{
    // Since this is a string literal, output it double quoted
    return "\"" + value + "\"";
}


Syntax::Production SpecificString::GetMyProduction() const
{ 
	return Production::TOKEN; 
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
    value(TypeDb::int_bits, false)  // 64-bit
{ 
    value = i; 
} 


SpecificInteger::SpecificInteger( unsigned i ) : 
    value(TypeDb::int_bits, true)  // unsigned
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
 
 
string SpecificInteger::GetToken() const 
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


Syntax::Production SpecificInteger::GetMyProduction() const
{ 
	return Production::TOKEN; 
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
 

string SpecificFloat::GetToken() const
{
    char hs[256];
    // generate hex float since it can be exact
    convertToHexString( hs, 0, false, llvm::APFloat::rmTowardNegative); // note rounding mode ignored when hex_digits==0
    return string(hs) +
           (&getSemantics()==TypeDb::float_semantics ? "F" : "") +
           (&getSemantics()==TypeDb::long_double_semantics ? "L" : "");
}


Syntax::Production SpecificFloat::GetMyProduction() const
{ 
	return Production::TOKEN; 
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


string SpecificIdentifier::GetToken() const 
{
    return name;
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

//////////////////////////// SpecificInstanceIdentifier //////////////////////////////

Syntax::Production SpecificInstanceIdentifier::GetMyProduction() const
{ 
	return Production::PURE_IDENTIFIER; 
}

//////////////////////////// SpecificTypeIdentifier //////////////////////////////

Syntax::Production SpecificTypeIdentifier::GetMyProduction() const
{ 
	return Production::PURE_IDENTIFIER; 
}

//////////////////////////// Instance //////////////////////////////

Syntax::Production Instance::GetMyProduction() const
{ 
	return Production::PROTOTYPE; 
}

//////////////////////////// SpecificLabelIdentifier //////////////////////////////

Syntax::Production SpecificLabelIdentifier::GetMyProduction() const
{ 
	return Production::PURE_IDENTIFIER; 
}

//////////////////////////// Label //////////////////////////////

Syntax::Production Label::GetMyProduction() const
{ 
	return Production::LABEL; 
}

//////////////////////////// Callable //////////////////////////////

Syntax::Production Callable::GetOperandInDeclaratorProduction() const
{
	return Production::POSTFIX; // eg int a();
}

//////////////////////////// Array //////////////////////////////

Syntax::Production Array::GetMyProduction() const
{ 
	return Production::POSTFIX; // eg auto a = new int[9]; (without booting)
}


Syntax::Production Array::GetOperandInDeclaratorProduction() const
{
	return Production::POSTFIX; // eg int a[9];
}

//////////////////////////// Indirection //////////////////////////////

Syntax::Production Indirection::GetMyProduction() const
{ 
	return Production::POSTFIX; // eg auto a = new int *; (without booting)
}


Syntax::Production Indirection::GetOperandInDeclaratorProduction() const
{
	return Production::PREFIX; // eg int *a;
}

//////////////////////////// Void ///////////////////////////////

Syntax::Production Void::GetMyProduction() const
{ 
	return Production::TOKEN; // eg auto a = new void;
}

//////////////////////////// Boolean ///////////////////////////////

Syntax::Production Boolean::GetMyProduction() const
{ 
	return Production::TOKEN; // eg auto a = new bool;
}

//////////////////////////// Numeric ///////////////////////////////

Syntax::Production Numeric::GetMyProduction() const
{ 
	return Production::SPACE_SEP_TYPE; // eg auto a = new unsigned long;
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

//////////////////////////// Labeley ///////////////////////////////

Syntax::Production Labeley::GetMyProduction() const
{
	return Production::POSTFIX; // renders as void *
}

//////////////////////////// True ///////////////////////////////

Syntax::Production True::GetMyProduction() const
{ 
	return Production::TOKEN; 
}

//////////////////////////// False ///////////////////////////////

Syntax::Production False::GetMyProduction() const
{ 
	return Production::TOKEN; 
}

//////////////////////////// New ///////////////////////////////

Syntax::Production New::GetMyProduction() const
{ 
	return Production::PREFIX; 
}

//////////////////////////// Delete ///////////////////////////////

Syntax::Production Delete::GetMyProduction() const
{ 
	return Production::PREFIX; 
}

//////////////////////////// Lookup ///////////////////////////////

Syntax::Production Lookup::GetMyProduction() const
{ 
	return Production::POSTFIX; 
}

//////////////////////////// Cast ///////////////////////////////

Syntax::Production Cast::GetMyProduction() const
{ 
	return Production::PREFIX; 
}

//////////////////////////// Call ///////////////////////////////

Syntax::Production Call::GetMyProduction() const
{ 
	return Production::POSTFIX; 
}

//////////////////////////// MakeRecord ///////////////////////////////

Syntax::Production MakeRecord::GetMyProduction() const
{ 
	return Production::PARENTHESISED; 
}

//////////////////////////// SizeOf ///////////////////////////////

Syntax::Production SizeOf::GetMyProduction() const
{ 
	return Production::PREFIX; 	
}

//////////////////////////// AlignOf ///////////////////////////////

Syntax::Production AlignOf::GetMyProduction() const
{ 
	return Production::PREFIX; 
}

//////////////////////////// Compound ///////////////////////////////

Syntax::Production Compound::GetMyProduction() const
{ 
	return Production::BRACED; 
}

//////////////////////////// StatementExpression ///////////////////////////////

Syntax::Production StatementExpression::GetMyProduction() const
{ 
	return Production::PARENTHESISED; 
}

//////////////////////////// Return ///////////////////////////////

Syntax::Production Return::GetMyProduction() const
{ 
	return Production::BARE_STATEMENT; 
}

//////////////////////////// Goto ///////////////////////////////

Syntax::Production Goto::GetMyProduction() const
{ 
	return Production::BARE_STATEMENT; 
}

//////////////////////////// If ///////////////////////////////

Syntax::Production If::GetMyProduction() const
{ 
	// If we don't have an else clause, we might steal the else from a 
	// surrounding If node, so drop our precedence a little bit.
	bool has_else_clause = !DynamicTreePtrCast<Nop>(else_body);
	return has_else_clause ? Production::STATEMENT_HIGH : Production::STATEMENT_LOW; 
}

//////////////////////////// Breakable ///////////////////////////////

Syntax::Production Breakable::GetMyProduction() const
{ 
	return Production::STATEMENT_HIGH; 
}

//////////////////////////// Do ///////////////////////////////

Syntax::Production Do::GetMyProduction() const
{ 
	return Production::BARE_STATEMENT; 
}

//////////////////////////// SwitchTarget ///////////////////////////////

Syntax::Production SwitchTarget::GetMyProduction() const
{ 
	return Production::LABEL; 
}

//////////////////////////// Continue ///////////////////////////////

Syntax::Production Continue::GetMyProduction() const
{ 
	return Production::BARE_STATEMENT; 
}

//////////////////////////// Break ///////////////////////////////

Syntax::Production Break::GetMyProduction() const
{ 
	return Production::BARE_STATEMENT; 
}

//////////////////////////// Nop ///////////////////////////////

Syntax::Production Nop::GetMyProduction() const
{ 
	return Production::BOOT_STATEMENT; // Force a {}
}

//////////////////////////// ExteriorCall ///////////////////////////////

Syntax::Production ExteriorCall::GetMyProduction() const
{ 
	return Production::POSTFIX; 
}

//////////////////////////// MacroCall ///////////////////////////////

Syntax::Production MacroCall::GetMyProduction() const
{ 
	return Production::STATEMENT; 
}

