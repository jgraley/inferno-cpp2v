
#include "cpptree.hpp"

#define EXPLICIT_BASE 0

using namespace CPPTree;

//////////////////////////// SpecificString ///////////////////////////////

SpecificString::SpecificString() 
{
} 


SpecificString::SpecificString( string s ) :
  	value(s) 
{
}


bool SpecificString::IsLocalMatch( const Matcher *candidate ) const 
{
    ASSERT( candidate );
    auto *c = dynamic_cast<const SpecificString *>(candidate);
    return c && c->value == value;
}

 
Orderable::Diff SpecificString::OrderCompare3WayLocal( const Orderable &right, 
                                                     OrderProperty order_property ) const
{
    auto &r = *GET_THAT_POINTER(&right);
    return value.compare(r.value);
}
 
 
string SpecificString::GetRender() const
{
    // Since this is a string literal, output it double quoted
    return "\"" + value + "\"";
}


string SpecificString::GetTrace() const
{
    return GetName() + "(" + GetRender() + ")" + GetSerialString();
}

//////////////////////////// SpecificInteger ///////////////////////////////

SpecificInteger::SpecificInteger()  : 
    value(INTEGER_DEFAULT_WIDTH, false)
{
}


SpecificInteger::SpecificInteger( llvm::APSInt i ) : 
    value(i)
{
}


SpecificInteger::SpecificInteger( int i ) : 
    value(INTEGER_DEFAULT_WIDTH, false)  // signed
{ 
    value = i; 
} 


SpecificInteger::SpecificInteger( int64_t i ) : 
    value(INTEGER_DEFAULT_WIDTH, false)  // 64-bit
{ 
    value = i; 
} 


SpecificInteger::SpecificInteger( unsigned i ) : 
    value(INTEGER_DEFAULT_WIDTH, true)  // unsigned
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


bool SpecificInteger::IsLocalMatch( const Matcher *candidate ) const
{
    ASSERT( candidate );
    auto *c = dynamic_cast<const SpecificInteger *>(candidate);
    // A local match will require all fields to match, not just the numerical value.
    return c && 
           c->value.isUnsigned() == value.isUnsigned() &&
           c->value.getBitWidth() == value.getBitWidth() &&
           c->value == value;
}


Orderable::Diff SpecificInteger::OrderCompare3WayLocal( const Orderable &right, 
                                                        OrderProperty order_property ) const
{
    auto &r = *GET_THAT_POINTER(&right);

    if( int d = ((int)value.isUnsigned() - (int)r.value.isUnsigned()) )
        return d;
    if( int d = (int)(value.getBitWidth()) - (int)(r.value.getBitWidth()) )
        return d;
    return (value > r.value) - (value < r.value);
    // Note: just subtracting could overflow
}
 
 
string SpecificInteger::GetRender() const /// Produce a string for debug
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


string SpecificInteger::GetTrace() const
{
    return GetName() + "(" + GetRender() + ")" + GetSerialString();
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


bool SpecificFloat::IsLocalMatch( const Matcher *candidate ) const 
{
    ASSERT( candidate );
    auto *c = dynamic_cast<const SpecificFloat *>(candidate);
    // A local match will require all fields to match, not just the numerical value.
    return c && 
    //    c->getSemantics() == getSemantics() && //TODO
    bitwiseIsEqual( *c );
}


Orderable::Diff SpecificFloat::OrderCompare3WayLocal( const Orderable &right, 
                                                    OrderProperty order_property ) const
{
    auto &r = *GET_THAT_POINTER(&right);
        
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
 

string SpecificFloat::GetRender() const
{
    char hs[256];
    // generate hex float since it can be exact
    convertToHexString( hs, 0, false, llvm::APFloat::rmTowardNegative); // note rounding mode ignored when hex_digits==0
    return string(hs) +
           (&getSemantics()==TypeDb::float_semantics ? "F" : "") +
           (&getSemantics()==TypeDb::long_double_semantics ? "L" : "");
}


string SpecificFloat::GetTrace() const
{
    return GetName() + "(" + GetRender() + ")" + GetSerialString();
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


bool SpecificIdentifier::IsLocalMatch( const Matcher *candidate ) const 
{
    ASSERT( candidate );
    return candidate == this;
}


Orderable::Diff SpecificIdentifier::OrderCompare3WayLocal( const Orderable &right, 
                                                           OrderProperty order_property ) const
{
    auto &r = *GET_THAT_POINTER(&right);
        
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


string SpecificIdentifier::GetRender() const 
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

//////////////////////////// SpecificFloatSemantics ///////////////////////////////

SpecificFloatSemantics::SpecificFloatSemantics() 
{
}


SpecificFloatSemantics::SpecificFloatSemantics( const llvm::fltSemantics *s ) : 
    value(s)
{
}


bool SpecificFloatSemantics::IsLocalMatch( const Matcher *candidate ) const
{
    ASSERT( candidate );
    auto *c = dynamic_cast<const SpecificFloatSemantics *>(candidate);
    return c && c->value == value;
}


Orderable::Diff SpecificFloatSemantics::OrderCompare3WayLocal( const Orderable &right, 
                                                               OrderProperty order_property ) const
{
    auto &r = *GET_THAT_POINTER(&right); 

    Orderable::Diff d;
    switch( order_property )
    {
    case STRICT:
        // Don't use any particular ordering apart from where the 
        // llvm::fltSemantics are being stored.
        d = (int)(value > r.value) - (int)(value < r.value);
        // Note: just subtracting could overflow
        break;
    case REPEATABLE:
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

