
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
    const SpecificString *c = dynamic_cast<const SpecificString *>(candidate);
    return c && c->value == value;
}

 
string SpecificString::GetRender() const
{
    // Since this is a string literal, output it double quoted
    return "\"" + value + "\"";
}


string SpecificString::GetTrace() const
{
    return GetName() + "(" + GetRender() + ")" + GetAddr();
}

//////////////////////////// SpecificInteger ///////////////////////////////

SpecificInteger::SpecificInteger() 
{
}


SpecificInteger::SpecificInteger( llvm::APSInt i ) : 
    llvm::APSInt(i)
{
}


SpecificInteger::SpecificInteger( int i ) : 
    llvm::APSInt(INTEGER_DEFAULT_WIDTH) 
{ 
    *(llvm::APSInt *)this = i; 
} 


bool SpecificInteger::IsLocalMatch( const Matcher *candidate ) const
{
    ASSERT( candidate );
    const SpecificInteger *c = dynamic_cast<const SpecificInteger *>(candidate);
    // A local match will require all fields to match, not just the numerical value.
    return c && 
           c->isUnsigned() == isUnsigned() &&
           c->getBitWidth() == getBitWidth() &&
           *(llvm::APSInt *)c == *(llvm::APSInt *)this;
}


string SpecificInteger::GetRender() const /// Produce a string for debug
{
    return string(toString(10)) + // decimal
           (isUnsigned() ? "U" : "") +
           (getBitWidth()>TypeDb::integral_bits[clang::DeclSpec::TSW_unspecified] ? "L" : "") +
           (getBitWidth()>TypeDb::integral_bits[clang::DeclSpec::TSW_long] ? "L" : "");
           // note, assuming longlong bigger than long, so second L appends first to get LL
}


string SpecificInteger::GetTrace() const
{
    return GetName() + "(" + GetRender() + ")" + GetAddr();
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
    const SpecificFloat *c = dynamic_cast<const SpecificFloat *>(candidate);
    // A local match will require all fields to match, not just the numerical value.
    return c && 
    //    c->getSemantics() == getSemantics() && //TODO
    bitwiseIsEqual( *c );
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
    return GetName() + "(" + GetRender() + ")" + GetAddr();
}

//////////////////////////// SpecificIdentifier ///////////////////////////////

SpecificIdentifier::SpecificIdentifier() 
{
}


SpecificIdentifier::SpecificIdentifier( string s ) : 
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


string SpecificIdentifier::GetRender() const 
{
    return name;
}


string SpecificIdentifier::GetTrace() const
{
    // Since this is text from the program, use single quotes
    return GetName() + "('" + GetRender() + "')" + GetAddr();
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
    const SpecificFloatSemantics *c = dynamic_cast<const SpecificFloatSemantics *>(candidate);
    return c && c->value == value;
}


SpecificFloatSemantics::operator const llvm::fltSemantics &() const 
{
    return *value;
}

