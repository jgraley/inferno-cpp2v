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

// ------------------------- SingleSymbolResult --------------------------

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


set<SR::XLink> SingleSymbolResult::GetAsSetOfXLinks() const
{
    return xlink ? set<SR::XLink>{ xlink } : set<SR::XLink>{};
}


bool SingleSymbolResult::operator==( const ResultInterface &other ) const
{
    auto o = dynamic_cast<const SingleSymbolResult *>(&other);
    return o && xlink == o->xlink;
}

// ------------------------- MultiSymbolResult --------------------------

MultiSymbolResult::MultiSymbolResult( set<SR::XLink> xlinks_ ) :
    xlinks( xlinks_ )
{
}


bool MultiSymbolResult::IsDefinedAndUnique() const
{
    return xlinks.size() == 1;
}


SR::XLink MultiSymbolResult::GetAsXLink() const
{
    return OnlyElementOf(xlinks);
}


set<SR::XLink> MultiSymbolResult::GetAsSetOfXLinks() const
{
    return xlinks;
}


bool MultiSymbolResult::operator==( const ResultInterface &other ) const
{
    auto o = dynamic_cast<const MultiSymbolResult *>(&other);
    return o && xlinks == o->xlinks;
}
