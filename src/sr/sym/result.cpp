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


set<SR::XLink> SymbolResult::GetAsSetOfXLinks() const
{
    return xlink ? set<SR::XLink>{ xlink } : set<SR::XLink>{};
}


bool SymbolResult::operator==( const ResultInterface &other ) const
{
    auto o = dynamic_cast<const SymbolResult *>(&other);
    return o && xlink == o->xlink;
}

// ------------------------- SymbolSetResult --------------------------

SymbolSetResult::SymbolSetResult( set<SR::XLink> xlinks_, bool complement_flag_ ) :
    xlinks( xlinks_ ),
    complement_flag( complement_flag_ )
{
}


static shared_ptr<SymbolSetResult> Create( shared_ptr<SymbolResultInterface> other )
{
    if( auto ssr = dynamic_pointer_cast<SymbolSetResult>(other) )
        return ssr;
    else if( auto sr = dynamic_pointer_cast<SymbolResult>(other) )
        return make_shared<SymbolSetResult>( other->GetAsSetOfXLinks() );
    else
        ASSERTFAIL();
}


bool SymbolSetResult::IsDefinedAndUnique() const
{
    return !complement_flag && xlinks.size() == 1;
}


SR::XLink SymbolSetResult::GetAsXLink() const
{
    ASSERT( !complement_flag )("Is complement so not unique");
    return OnlyElementOf(xlinks);
}


set<SR::XLink> SymbolSetResult::GetAsSetOfXLinks() const
{
    ASSERT( !complement_flag )("Refusing to extensionalise a complement set");
    return xlinks;
}


bool SymbolSetResult::operator==( const ResultInterface &other ) const
{
    auto o = dynamic_cast<const SymbolSetResult *>(&other);
    return o && xlinks == o->xlinks && complement_flag==o->complement_flag;
}


shared_ptr<SymbolSetResult> SymbolSetResult::GetComplement() const
{
    return make_shared<SymbolSetResult>(xlinks, !complement_flag);
}

