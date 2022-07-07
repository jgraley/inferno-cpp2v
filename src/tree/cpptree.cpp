
#include "cpptree.hpp"

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

 
Orderable::Result SpecificString::OrderCompareLocal( const Orderable *candidate, 
                                                     OrderProperty order_property ) const
{
    auto c = GET_THAT_POINTER(candidate);
    return value.compare(c->value);
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

SpecificInteger::SpecificInteger() 
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


Orderable::Result SpecificInteger::OrderCompareLocal( const Orderable *candidate, 
                                                      OrderProperty order_property ) const
{
    auto c = GET_THAT_POINTER(candidate);

    if( value.isUnsigned() != c->value.isUnsigned() )
        return (int)(value.isUnsigned()) - (int)(c->value.isUnsigned());
    if( value.getBitWidth() != c->value.getBitWidth() )
        return (int)(value.getBitWidth()) - (int)(c->value.getBitWidth());
    return (value > c->value) - (value < c->value);
    // Note: just subtracting could overflow
}
 
 
string SpecificInteger::GetRender() const /// Produce a string for debug
{
    return string(value.toString(10)) + // decimal
           (value.isUnsigned() ? "U" : "") +
           (value.getBitWidth()>TypeDb::integral_bits[clang::DeclSpec::TSW_unspecified] ? "L" : "") +
           (value.getBitWidth()>TypeDb::integral_bits[clang::DeclSpec::TSW_long] ? "L" : "");
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


Orderable::Result SpecificFloat::OrderCompareLocal( const Orderable *candidate, 
                                                    OrderProperty order_property ) const
{
    auto c = GET_THAT_POINTER(candidate);
        
    // Primary ordering: the value
    cmpResult cr = compare(*c);
    if( cr==APFloat::cmpLessThan )
        return -1;
    if( cr==APFloat::cmpGreaterThan )
        return 1;    
    
    // Secondary ordering: the hash
    uint32_t h = getHashValue();
    uint32_t ch = c->getHashValue();
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

SpecificIdentifier::SpecificIdentifier() 
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


Orderable::Result SpecificIdentifier::OrderCompareLocal( const Orderable *candidate, 
                                                         OrderProperty order_property ) const
{
    auto c = GET_THAT_POINTER(candidate);
        
    //FTRACEC("Compare ")(*this)(" with ")(*c)(": ");

    if( c == this )
    {
        //FTRACEC("0 (fast out)\n");
        return Orderable::EQUAL; // fast-out
    }
        
    // Primary ordering on name due rule #528
    if( name != c->name )
    {
        //FTRACEC("%d (name)\n", name.compare(c->name));
        return name.compare(c->name);      
    }
          
    // Optional over-ride of address compare for making ranges, see rule #528
    if( addr_bounding_role != BoundingRole::NONE || c->addr_bounding_role != BoundingRole::NONE )
    {
        return (int)addr_bounding_role - (int)(c->addr_bounding_role);
    }    
    
    // Secondary ordering on address due rule #528
    Orderable::Result r;
    switch( order_property )
    {
    case STRICT:
        // Unique order uses address to ensure different identifiers compare differently
        r = (int)(this > candidate) - (int)(this < candidate);
        // Note: just subtracting could overflow
        break;
    case REPEATABLE:
        // Repeatable ordering stops after name check since address compare is not repeatable
        r = Orderable::EQUAL;
        break;
    }
    //FTRACEC("%d (address)\n", r);
    return r;
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


Orderable::Result SpecificFloatSemantics::OrderCompareLocal( const Orderable *candidate, 
                                                             OrderProperty order_property ) const
{
    auto c = GET_THAT_POINTER(candidate); 

    Orderable::Result r;
    switch( order_property )
    {
    case STRICT:
        // Don't use any particular ordering apart from where the 
        // llvm::fltSemantics are being stored.
        r = (int)(value > c->value) - (int)(value < c->value);
        // Note: just subtracting could overflow
        break;
    case REPEATABLE:
        // Repeatable ordering stops at type
        r = Orderable::EQUAL;
        break;
    }
    return r;
}


SpecificFloatSemantics::operator const llvm::fltSemantics &() const 
{
    return *value;
}

