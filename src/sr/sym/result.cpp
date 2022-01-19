#include "result.hpp"
#include "primary_expressions.hpp"

using namespace SYM;

// ------------------------- BooleanResult --------------------------

BooleanResult::BooleanResult( Category cat, bool value )
{
    switch( cat )
    {
    case UNDEFINED:
        ASSERT( !value );
        certainty = Certainty::UNDEFINED;
        break;
    case DEFINED:        
        certainty = value ? Certainty::TRUE : Certainty::FALSE;
        break;
    default:
        ASSERTFAIL("Missing case");
    }
}


bool BooleanResult::IsDefinedAndUnique() const
{
    return certainty != Certainty::UNDEFINED;
}


bool BooleanResult::IsDefinedAndTrue() const
{
    return certainty == Certainty::TRUE;
}


bool BooleanResult::IsDefinedAndFalse() const
{
    return certainty == Certainty::FALSE;
}

   
bool BooleanResult::GetAsBool() const
{
    ASSERT( IsDefinedAndUnique() );
    return certainty == Certainty::TRUE;
}


bool BooleanResult::operator<( const BooleanResult &other ) const
{
    return certainty < other.certainty;
}


bool BooleanResult::CertaintyCompare( const shared_ptr<BooleanResult> &a, 
                                      const shared_ptr<BooleanResult> &b )
{
    return *a < *b;
}                                      

// ------------------------- SymbolResult --------------------------

SymbolResult::SymbolResult( Category cat, SR::XLink xlink_ )
{
    switch( cat )
    {
    case UNDEFINED:
        ASSERT( !xlink_ );
        break;
    case DEFINED:
        ASSERT( xlink_ );
        xlink = xlink_;
        break;
    default:
        ASSERTFAIL("Missing case");
    }
}


bool SymbolResult::IsDefinedAndUnique() const
{
    return (bool)xlink;
}


SR::XLink SymbolResult::GetAsXLink() const
{
    ASSERT( xlink );
    return xlink;
}
