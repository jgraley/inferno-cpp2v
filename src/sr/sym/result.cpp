#include "result.hpp"
#include "primary_expressions.hpp"

using namespace SYM;

// ------------------------- BooleanResultInterface --------------------------

bool BooleanResultInterface::CertaintyCompare( const shared_ptr<BooleanResultInterface> &a, 
                                               const shared_ptr<BooleanResultInterface> &b )
{
    return *a < *b;
}                                      

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


bool BooleanResult::operator==( const ResultInterface &other ) const
{
    auto o = dynamic_cast<const BooleanResult *>(&other);
    return o && certainty == o->certainty;
}


bool BooleanResult::operator<( const BooleanResultInterface &other ) const
{
    auto o = dynamic_cast<const BooleanResult *>(&other);
    ASSERT(o);
    return certainty < o->certainty;
}

// ------------------------- SymbolResult --------------------------

SingleSymbolResult::SingleSymbolResult( Category cat, SR::XLink xlink_ )
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


bool SingleSymbolResult::IsDefinedAndUnique() const
{
    return (bool)xlink;
}


SR::XLink SingleSymbolResult::GetAsXLink() const
{
    ASSERT( xlink );
    return xlink;
}


bool SingleSymbolResult::operator==( const ResultInterface &other ) const
{
    auto o = dynamic_cast<const SingleSymbolResult *>(&other);
    return o && xlink == o->xlink;
}


// ------------------------- SymbolSetResult --------------------------
/*
SymbolSetResult::SymbolSetResult( set<SR::XLink> xlinks_ ) :
    xlinks( xlinks_ )
{
}


bool SymbolSetResult::IsDefinedAndUnique() const
{
    return xlinks.size() == 1;
}


SR::XLink SymbolSetResult::GetAsXLink() const
{
    return OnlyElementOf(xlink);
}


const set<SR::XLink> &SymbolSetResult::GetAsSet() const
{
    return xlinks;
}
*/